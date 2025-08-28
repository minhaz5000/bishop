#ifndef BISHOP_H_
#define BISHOP_H_

#include <string>

#include "nfa.hpp"
#include "tokenizer.hpp"

namespace bishop {

class Regex {
public:
    // Constructors
    Regex() = default;

    explicit Regex(const std::string& pattern)
    {
        compile(pattern);
    }

    // Compile a new regex pattern
    void compile(const std::string& pattern)
    {
        pattern_ = pattern;
        auto tokens = tokenizer::to_postfix(tokenizer::insert_concats(tokenizer::tokenize(pattern_)));
        nfa_ = nfa::parse_postfix(tokens);
    }

    // Match a string against the compiled regex
    bool match(const std::string& text)
    {
        return nfa::nfa_match(nfa_, text);
    }

private:
    std::string pattern_;
    nfa::NFA nfa_;
};
}  // namespace bishop

#endif
