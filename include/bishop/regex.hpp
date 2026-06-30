#ifndef BISHOP_H_
#define BISHOP_H_

#include <string>
#include <utility>
#include <vector>

#include "bishop/error.hpp"
#include "bishop/flags.hpp"
#include "bishop/nfa.hpp"
#include "bishop/tokenizer.hpp"

namespace bishop {

class Regex {
public:
    Regex() = default;

    explicit Regex(const std::string& pattern, Flag flags = Flag::NONE)
    {
        compile(pattern, flags);
    }

    Regex(const Regex& other)
        : flags_(other.flags_),
          anchored_start_(other.anchored_start_),
          anchored_end_(other.anchored_end_),
          pattern_(other.pattern_)
    {
        if (other.nfa_.start) {
            int id = 0;
            nfa_ = nfa::clone_nfa(id, other.nfa_);
        }
    }

    Regex& operator=(const Regex& other)
    {
        if (this == &other) return *this;

        nfa::NFA cloned{nullptr, nullptr};
        if (other.nfa_.start) {
            int id = 0;
            cloned = nfa::clone_nfa(id, other.nfa_);
        }

        if (nfa_.start) nfa::free_nfa(nfa_);
        nfa_ = cloned;
        flags_ = other.flags_;
        anchored_start_ = other.anchored_start_;
        anchored_end_ = other.anchored_end_;
        pattern_ = other.pattern_;
        return *this;
    }

    Regex(Regex&& other) noexcept
        : nfa_(other.nfa_),
          flags_(other.flags_),
          anchored_start_(other.anchored_start_),
          anchored_end_(other.anchored_end_),
          pattern_(std::move(other.pattern_))
    {
        other.nfa_ = {nullptr, nullptr};
    }

    Regex& operator=(Regex&& other) noexcept
    {
        if (this == &other) return *this;

        if (nfa_.start) nfa::free_nfa(nfa_);
        nfa_ = other.nfa_;
        flags_ = other.flags_;
        anchored_start_ = other.anchored_start_;
        anchored_end_ = other.anchored_end_;
        pattern_ = std::move(other.pattern_);
        other.nfa_ = {nullptr, nullptr};
        return *this;
    }

    ~Regex()
    {
        if (nfa_.start) nfa::free_nfa(nfa_);
    }

    void compile(const std::string& pattern, Flag flags = Flag::NONE)
    {
        if (nfa_.start) {
            nfa::free_nfa(nfa_);
        }
        try {
            flags_ = flags;
            auto tokens = tokenizer::tokenize(pattern, flags);
            auto [anc_s, anc_e] = tokenizer::strip_anchors(tokens);
            anchored_start_ = anc_s;
            anchored_end_ = anc_e;
            if (has_flag(flags, Flag::IGNORECASE)) tokenizer::apply_ignorecase(tokens);
            tokens = tokenizer::insert_concats(tokens);
            auto postfix = tokenizer::to_postfix(tokens);
            nfa_ = nfa::parse_postfix(postfix);
            pattern_ = pattern;
        }
        catch (const RegexError&) {
            throw;
        }
        catch (const std::exception& e) {
            throw CompileError(std::string("unexpected error: ") + e.what());
        }
    }

    bool match(const std::string& text) const
    {
        return nfa::nfa_match(nfa_, text, flags_);
    }

    std::pair<size_t, size_t> search(const std::string& text)
    {
        size_t start_hi = anchored_start_ ? 1 : text.size() + 1;
        for (size_t i = 0; i < start_hi; ++i) {
            size_t j_lo = anchored_end_ ? text.size() : i;
            for (size_t j = j_lo; j <= text.size(); ++j) {
                if (nfa::nfa_match(nfa_, text.substr(i, j - i), flags_)) return {i, j};
            }
        }
        return {std::string::npos, std::string::npos};
    }

    std::vector<std::pair<size_t, size_t>> find_all(const std::string& text)
    {
        std::vector<std::pair<size_t, size_t>> results;
        size_t pos = 0;
        while (pos <= text.size()) {
            bool found = false;
            for (size_t j = text.size(); j >= pos; --j) {
                if (nfa::nfa_match(nfa_, text.substr(pos, j - pos), flags_)) {
                    results.push_back({pos, j});
                    pos = (j == pos) ? j + 1 : j;
                    found = true;
                    break;
                }
                if (j == pos) break;
            }
            if (!found) ++pos;
        }
        return results;
    }

    std::string replace(const std::string& text, const std::string& repl)
    {
        auto [s, e] = search(text);
        if (s == std::string::npos) return text;
        return text.substr(0, s) + repl + text.substr(e);
    }

    std::string replace_all(const std::string& text, const std::string& repl)
    {
        auto matches = find_all(text);
        if (matches.empty()) return text;
        std::string result;
        size_t pos = 0;
        for (auto& [s, e] : matches) {
            result += text.substr(pos, s - pos);
            result += repl;
            pos = e;
        }
        result += text.substr(pos);
        return result;
    }

    const std::string& pattern() const noexcept
    {
        return pattern_;
    }
    Flag flags() const noexcept
    {
        return flags_;
    }
    bool anchored_start() const noexcept
    {
        return anchored_start_;
    }
    bool anchored_end() const noexcept
    {
        return anchored_end_;
    }

    nfa::NFA& nfa_ref() noexcept
    {
        return nfa_;
    }

    nfa::MatchData search_match(const std::string& text)
    {
        size_t start_hi = anchored_start_ ? 1 : text.size() + 1;
        for (size_t i = 0; i < start_hi; ++i) {
            size_t j_lo = anchored_end_ ? text.size() : i;
            for (size_t j = j_lo; j <= text.size(); ++j) {
                auto md = nfa::nfa_match_captures(nfa_, text.substr(i, j - i), 0, flags_);
                if (md.matched) {
                    md.start = i;
                    md.end = j;
                    return md;
                }
            }
        }
        return {};
    }

    std::vector<nfa::MatchData> find_all_matches(const std::string& text)
    {
        std::vector<nfa::MatchData> results;
        size_t pos = 0;
        while (pos <= text.size()) {
            bool found = false;
            for (size_t j = text.size(); j >= pos; --j) {
                auto md = nfa::nfa_match_captures(nfa_, text.substr(pos, j - pos), 0, flags_);
                if (md.matched) {
                    md.start = pos;
                    md.end = j;
                    results.push_back(md);
                    pos = (j == pos) ? j + 1 : j;
                    found = true;
                    break;
                }
                if (j == pos) break;
            }
            if (!found) ++pos;
        }
        return results;
    }

private:
    nfa::NFA nfa_{nullptr, nullptr};
    Flag flags_{Flag::NONE};
    bool anchored_start_{false};
    bool anchored_end_{false};
    std::string pattern_;
};

}  // namespace bishop
#endif
