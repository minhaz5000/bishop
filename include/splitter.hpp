#ifndef BISHOP_SPLITTER_HPP_
#define BISHOP_SPLITTER_HPP_

#include <string>
#include <vector>

#include "bishop/flags.hpp"
#include "bishop/nfa.hpp"
#include "bishop/regex.hpp"

namespace bishop {
namespace splitter {

struct SplitSegment {
    std::string text;
    bool is_delimiter{false};
    size_t offset{0};
};

class Splitter {
public:
    explicit Splitter(const std::string& pattern, Flag flags = Flag::NONE);

    explicit Splitter(Regex re);

    std::vector<std::string> split(const std::string& text) const;

    std::vector<std::string> split_n(const std::string& text, size_t limit) const;

    std::vector<SplitSegment> split_with_delimiters(const std::string& text) const;

    static std::string join(const std::vector<SplitSegment>& segments);

    static std::string join(const std::vector<std::string>& parts, const std::string& sep);

    const Regex& regex() const noexcept
    {
        return re_;
    }

private:
    std::pair<size_t, size_t> find_delimiter(const std::string& text, size_t pos) const;

    Regex re_;
};

std::vector<std::string> split(const std::string& text, const std::string& pattern, Flag flags = Flag::NONE);

std::vector<std::string> split_n(const std::string& text, const std::string& pattern, size_t limit,
                                 Flag flags = Flag::NONE);

std::vector<SplitSegment> split_with_delimiters(const std::string& text, const std::string& pattern,
                                                Flag flags = Flag::NONE);

}  // namespace splitter
}  // namespace bishop
#endif  // BISHOP_SPLITTER_HPP_
