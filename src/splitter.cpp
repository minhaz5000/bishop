#include "bishop/splitter.hpp"

#include <algorithm>
#include <stdexcept>

namespace bishop {
namespace splitter {

Splitter::Splitter(const std::string& pattern, Flag flags) : re_(pattern, flags) {}

Splitter::Splitter(Regex re) : re_(std::move(re)) {}

std::pair<size_t, size_t> Splitter::find_delimiter(const std::string& text, size_t pos) const
{
    for (size_t i = pos; i <= text.size(); ++i) {
        for (size_t j = text.size(); j >= i; --j) {
            auto md = nfa::nfa_match_captures(const_cast<Regex&>(re_).nfa_ref(), text.substr(i, j - i), 0, re_.flags());
            if (md.matched) return {i, j};
            if (j == i) break;
        }
    }
    return {std::string::npos, std::string::npos};
}

std::vector<std::string> Splitter::split(const std::string& text) const
{
    return split_n(text, 0);
}

std::vector<std::string> Splitter::split_n(const std::string& text, size_t limit) const
{
    std::vector<std::string> parts;
    size_t pos = 0;

    while (pos <= text.size()) {
        if (limit > 0 && parts.size() + 1 >= limit) {
            parts.push_back(text.substr(pos));
            return parts;
        }

        auto [ds, de] = find_delimiter(text, pos);

        if (ds == std::string::npos) {
            parts.push_back(text.substr(pos));
            break;
        }

        parts.push_back(text.substr(pos, ds - pos));

        pos = (de > ds) ? de : ds + 1;
    }

    return parts;
}

std::vector<SplitSegment> Splitter::split_with_delimiters(const std::string& text) const
{
    std::vector<SplitSegment> segs;
    size_t pos = 0;

    while (pos <= text.size()) {
        auto [ds, de] = find_delimiter(text, pos);

        if (ds == std::string::npos) {
            SplitSegment seg;
            seg.text = text.substr(pos);
            seg.is_delimiter = false;
            seg.offset = pos;
            segs.push_back(std::move(seg));
            break;
        }

        {
            SplitSegment seg;
            seg.text = text.substr(pos, ds - pos);
            seg.is_delimiter = false;
            seg.offset = pos;
            segs.push_back(std::move(seg));
        }

        {
            SplitSegment seg;
            seg.text = text.substr(ds, de - ds);
            seg.is_delimiter = true;
            seg.offset = ds;
            segs.push_back(std::move(seg));
        }

        pos = (de > ds) ? de : ds + 1;
    }

    return segs;
}

std::string Splitter::join(const std::vector<SplitSegment>& segments)
{
    std::string result;
    for (const auto& seg : segments) result += seg.text;
    return result;
}

std::string Splitter::join(const std::vector<std::string>& parts, const std::string& sep)
{
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) result += sep;
        result += parts[i];
    }
    return result;
}

std::vector<std::string> split(const std::string& text, const std::string& pattern, Flag flags)
{
    Splitter sp(pattern, flags);
    return sp.split(text);
}

std::vector<std::string> split_n(const std::string& text, const std::string& pattern, size_t limit, Flag flags)
{
    Splitter sp(pattern, flags);
    return sp.split_n(text, limit);
}

std::vector<SplitSegment> split_with_delimiters(const std::string& text, const std::string& pattern, Flag flags)
{
    Splitter sp(pattern, flags);
    return sp.split_with_delimiters(text);
}

}  // namespace splitter
}  // namespace bishop
