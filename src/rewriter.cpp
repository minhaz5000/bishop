#include "bishop/rewriter.hpp"

#include <cctype>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace bishop {
namespace rewriter {

std::vector<ReplacePart> parse_replacement(const std::string& repl)
{
    std::vector<ReplacePart> parts;
    std::string lit;

    auto flush_lit = [&]() {
        if (!lit.empty()) {
            ReplacePart p;
            p.kind = ReplacePart::Kind::Literal;
            p.text = std::move(lit);
            parts.push_back(std::move(p));
            lit.clear();
        }
    };

    for (size_t i = 0; i < repl.size(); ++i) {
        const char ch = repl[i];

        if (ch == '\\' && i + 1 < repl.size()) {
            const char next = repl[i + 1];

            if (next == '\\') {
                lit += '\\';
                ++i;
                continue;
            }

            if (next == '0' || next == '&') {
                flush_lit();
                ReplacePart p;
                p.kind = ReplacePart::Kind::WholeMatch;
                parts.push_back(std::move(p));
                ++i;
                continue;
            }

            if (next >= '1' && next <= '9') {
                flush_lit();
                ReplacePart p;
                p.kind = ReplacePart::Kind::GroupIndex;
                p.index = next - '0';
                parts.push_back(std::move(p));
                ++i;
                continue;
            }

            lit += ch;
            continue;
        }

        if (ch == '$' && i + 1 < repl.size() && repl[i + 1] == '{') {
            size_t j = i + 2;
            while (j < repl.size() && repl[j] != '}') ++j;

            if (j < repl.size()) {
                std::string inner = repl.substr(i + 2, j - i - 2);
                flush_lit();

                if (inner == "0") {
                    ReplacePart p;
                    p.kind = ReplacePart::Kind::WholeMatch;
                    parts.push_back(std::move(p));
                } else {
                    bool all_digits = !inner.empty();
                    for (char c : inner)
                        if (!std::isdigit(static_cast<unsigned char>(c))) {
                            all_digits = false;
                            break;
                        }

                    if (all_digits) {
                        ReplacePart p;
                        p.kind = ReplacePart::Kind::GroupIndex;
                        p.index = std::stoi(inner);
                        parts.push_back(std::move(p));
                    } else {
                        ReplacePart p;
                        p.kind = ReplacePart::Kind::GroupName;
                        p.name = inner;
                        parts.push_back(std::move(p));
                    }
                }

                i = j;
                continue;
            }
        }

        lit += ch;
    }

    flush_lit();
    return parts;
}

std::string apply_replacement(const std::vector<ReplacePart>& parts, const nfa::MatchData& md, const std::string& input)
{
    std::string out;
    out.reserve(64);

    for (const ReplacePart& p : parts) {
        switch (p.kind) {
            case ReplacePart::Kind::Literal:
                out += p.text;
                break;

            case ReplacePart::Kind::WholeMatch:
                if (md.matched && md.start != std::string::npos && md.end != std::string::npos &&
                    md.end <= input.size())
                    out += input.substr(md.start, md.end - md.start);
                break;

            case ReplacePart::Kind::GroupIndex: {
                size_t gi = static_cast<size_t>(p.index - 1);
                if (md.group_matched(gi)) {
                    auto [gs, ge] = md.groups[gi];
                    if (ge <= input.size()) out += input.substr(gs, ge - gs);
                }
                break;
            }

            case ReplacePart::Kind::GroupName:
                break;
        }
    }

    return out;
}

std::string replace_first(Regex& re, const std::string& text, const std::string& repl)
{
    const auto parts = parse_replacement(repl);

    for (size_t i = 0; i <= text.size(); ++i) {
        for (size_t j = text.size(); j >= i; --j) {
            auto md = nfa::nfa_match_captures(re.nfa_ref(), text.substr(i, j - i), 0, re.flags());
            if (md.matched) {
                md.start = i;
                md.end = j;
                std::string result = text.substr(0, i);
                result += apply_replacement(parts, md, text);
                result += text.substr(j);
                return result;
            }
            if (j == i) break;
        }
    }

    return text;
}

std::string replace_all(Regex& re, const std::string& text, const std::string& repl)
{
    const auto parts = parse_replacement(repl);
    std::string result;
    result.reserve(text.size());

    size_t pos = 0;
    while (pos <= text.size()) {
        bool found = false;
        for (size_t j = text.size(); j >= pos; --j) {
            auto md = nfa::nfa_match_captures(re.nfa_ref(), text.substr(pos, j - pos), 0, re.flags());
            if (md.matched) {
                md.start = pos;
                md.end = j;
                result += apply_replacement(parts, md, text);
                pos = (j == pos) ? j + 1 : j;
                if (j == pos && pos <= text.size()) {
                    if (pos < text.size()) result += text[pos - 1];
                }
                found = true;
                break;
            }
            if (j == pos) break;
        }
        if (!found) {
            if (pos < text.size()) result += text[pos];
            ++pos;
        }
    }

    return result;
}

std::string replace_fn(Regex& re, const std::string& text,
                       const std::function<std::string(const nfa::MatchData&, const std::string&)>& fn)
{
    std::string result;
    result.reserve(text.size());

    size_t pos = 0;
    while (pos <= text.size()) {
        bool found = false;
        for (size_t j = text.size(); j >= pos; --j) {
            auto md = nfa::nfa_match_captures(re.nfa_ref(), text.substr(pos, j - pos), 0, re.flags());
            if (md.matched) {
                md.start = pos;
                md.end = j;
                result += fn(md, text);
                pos = (j == pos) ? j + 1 : j;
                found = true;
                break;
            }
            if (j == pos) break;
        }
        if (!found) {
            if (pos < text.size()) result += text[pos];
            ++pos;
        }
    }

    return result;
}

std::string dump_replacement(const std::vector<ReplacePart>& parts)
{
    std::ostringstream oss;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) oss << ", ";
        const auto& p = parts[i];
        switch (p.kind) {
            case ReplacePart::Kind::Literal:
                oss << "Literal(\"" << p.text << "\")";
                break;
            case ReplacePart::Kind::WholeMatch:
                oss << "WholeMatch";
                break;
            case ReplacePart::Kind::GroupIndex:
                oss << "Group(" << p.index << ")";
                break;
            case ReplacePart::Kind::GroupName:
                oss << "Named(\"" << p.name << "\")";
                break;
        }
    }
    return oss.str();
}

}  // namespace rewriter
}  // namespace bishop
