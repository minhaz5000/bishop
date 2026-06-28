#ifndef BISHOP_REWRITER_HPP_
#define BISHOP_REWRITER_HPP_

#include <functional>
#include <string>
#include <vector>

#include "bishop/nfa.hpp"
#include "bishop/regex.hpp"

namespace bishop {
namespace rewriter {

struct ReplacePart {
    enum class Kind {
        Literal,
        WholeMatch,
        GroupIndex,
        GroupName,
    };

    Kind kind{Kind::Literal};
    std::string text;
    int index{0};
    std::string name;
};

std::vector<ReplacePart> parse_replacement(const std::string& repl);

std::string apply_replacement(const std::vector<ReplacePart>& parts, const nfa::MatchData& md,
                              const std::string& input);

std::string replace_first(Regex& re, const std::string& text, const std::string& repl);

std::string replace_all(Regex& re, const std::string& text, const std::string& repl);

std::string replace_fn(Regex& re, const std::string& text,
                       const std::function<std::string(const nfa::MatchData&, const std::string&)>& fn);

std::string dump_replacement(const std::vector<ReplacePart>& parts);

}  // namespace rewriter
}  // namespace bishop
#endif  // BISHOP_REWRITER_HPP_
