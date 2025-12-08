#include "bishop/flags.hpp"
#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace tokenizer {

enum class TokenType {
    Literal, Union, Star, Plus, Question,
    LParen, RParen, Concat, Dot, CharClass, Repeat,
    AnchorStart, AnchorEnd,
    GroupStart, GroupEnd, NamedGroup, Unknown
};

struct GroupInfo {
    int         index{-1};
    std::string name;
};

struct Token {
    TokenType         type;
    char              value;
    std::vector<bool> char_set;
    int               repeat_min{0};
    int               repeat_max{-1};
    GroupInfo         group;
};

struct AnchorInfo {
    bool start;
    bool end;
};

constexpr int precedence_of(TokenType t) {
    switch (t) {
        case TokenType::Star:
        case TokenType::Plus:
        case TokenType::Question:
        case TokenType::Repeat:   return 3;
        case TokenType::Concat:   return 2;
        case TokenType::Union:    return 1;
        default:                  return 0;
    }
}

inline std::string to_string(const TokenType t) {
    switch (t) {
        case TokenType::Literal:     return "Literal";
        case TokenType::Plus:        return "Plus";
        case TokenType::Star:        return "Star";
        case TokenType::Question:    return "Question";
        case TokenType::LParen:      return "LParen";
        case TokenType::RParen:      return "RParen";
        case TokenType::Union:       return "Union";
        case TokenType::Concat:      return "Concat";
        case TokenType::Dot:         return "Dot";
        case TokenType::CharClass:   return "CharClass";
        case TokenType::Repeat:      return "Repeat";
        case TokenType::AnchorStart: return "AnchorStart";
        case TokenType::AnchorEnd:   return "AnchorEnd";
        case TokenType::GroupStart: return "GroupStart";
        case TokenType::GroupEnd:   return "GroupEnd";
        case TokenType::NamedGroup: return "NamedGroup";
        default:                     return "Unknown";
    }
}

inline AnchorInfo strip_anchors(std::vector<Token>& toks) {
    bool s = !toks.empty() && toks.front().type == TokenType::AnchorStart;
    bool e = !toks.empty() && toks.back().type  == TokenType::AnchorEnd;
    if (s) toks.erase(toks.begin());
    if (e) toks.pop_back();
    return {s, e};
}

std::vector<Token> tokenize(const std::string& s,
                             bishop::Flag flags = bishop::Flag::NONE);
void               apply_ignorecase(std::vector<Token>& tokens);
std::vector<Token> insert_concats(const std::vector<Token>& s);
std::vector<Token> to_postfix(const std::vector<Token>& s);

std::ostream& operator<<(std::ostream& os, const Token& t);

}  // namespace tokenizer
#endif
