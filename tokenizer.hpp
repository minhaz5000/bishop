#ifndef TOKENIZER_H_
#define TOKENIZER_H_    

#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace tokenizer {

enum class TokenType { Literal, Union, Star, Plus, Question, LParen, RParen, Concat, Unknown };
struct Token {
    TokenType type;
    char value;
};

constexpr int precedence_of(TokenType t) {
    switch (t) {
        case TokenType::Star:
        case TokenType::Plus:
        case TokenType::Question: return 3;
        case TokenType::Concat:   return 2;
        case TokenType::Union:    return 1;
        default:                  return 0;
    }
}

inline std::string to_string(const TokenType t)
{
    switch (t) {
        case TokenType::Literal:
            return "Literal";
        case TokenType::Plus:
            return "Plus";
        case TokenType::Star:
            return "Star";
        case TokenType::Question:
            return "Question";
        case TokenType::LParen:
            return "LParen";
        case TokenType::RParen:
            return "RParen";
        case TokenType::Union:
            return "Union";
        case TokenType::Concat:
            return "Concat";
        default:
            return "Unknown";
    }
}

std::vector<Token> tokenize(const std::string& s);
std::vector<Token> insert_concats(const std::vector<Token>& s);
std::vector<Token> to_postfix(const std::vector<Token>& s);

std::string TokenType_to_str(const TokenType t);
std::ostream& operator<<(std::ostream& os, const Token& t);

}  // namespace tokenizer
//
#endif
