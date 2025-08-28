#include "tokenizer.hpp"

#include <cctype>
#include <stack>

namespace tokenizer {
std::vector<Token> tokenize(const std::string& s)
{
    if (s.empty()) {
        return {};
    }
    std::vector<Token> tokens;
    tokens.reserve(s.size());

    auto it = s.cbegin();
    while (it != s.cend()) {
        char ch = static_cast<unsigned char>(*it);

        if (std::isalnum(ch)) {
            tokens.push_back({TokenType::Literal, ch});
        } else {
            switch (ch) {
                case '+':
                    tokens.push_back({TokenType::Plus, ch});
                    break;
                case '*':
                    tokens.push_back({TokenType::Star, ch});
                    break;
                case '?':
                    tokens.push_back({TokenType::Question, ch});
                    break;
                case '|':
                    tokens.push_back({TokenType::Union, ch});
                    break;
                case '(':
                    tokens.push_back({TokenType::LParen, ch});
                    break;
                case ')':
                    tokens.push_back({TokenType::RParen, ch});
                    break;
                default:
                    tokens.push_back({TokenType::Unknown, ch});
                    break;
            }
        }
        ++it;
    }
    return tokens;
}

std::vector<Token> insert_concats(const std::vector<Token>& s)
{
    if (s.empty()) {
        return {};
    }

    std::vector<Token> tokens;
    tokens.reserve(s.size() * 2);

    for (size_t i = 0; i < s.size(); ++i) {
        tokens.push_back(s[i]);

        if (i + 1 >= s.size()) break;

        const auto& curr = s[i];
        const auto& next = s[i + 1];

        // if the curr is any of these tokens then concat is maybe allowed
        bool curr_allows_concat = curr.type == TokenType::Literal || curr.type == TokenType::RParen ||
                                  curr.type == TokenType::Star || curr.type == TokenType::Plus ||
                                  curr.type == TokenType::Question;

        bool next_allows_concat = next.type == TokenType::Literal || next.type == TokenType::LParen;

        if (curr_allows_concat && next_allows_concat) {
            tokens.push_back({TokenType::Concat, '.'});
        }
    }

    return tokens;
}

std::ostream& operator<<(std::ostream& os, const Token& t)
{
    os << "[" << to_string(t.type) << ": " << t.value << "]";
    return os;
}

std::vector<Token> to_postfix(const std::vector<Token>& s)
{
    if (s.empty()) return {};

    std::stack<Token> operator_st;
    std::vector<Token> out;
    out.reserve(s.size() * 2);

    for (const auto& tok : s) {
        switch (tok.type) {
            case TokenType::Literal:
                out.push_back(tok);
                break;
            case TokenType::LParen:
                operator_st.push(tok);
                break;
            case TokenType::RParen:
                while (!operator_st.empty() && operator_st.top().type != TokenType::LParen) {
                    out.push_back(operator_st.top());
                    operator_st.pop();
                }
                if (!operator_st.empty()) operator_st.pop();  // discard '('
                break;
            default:  // operators: *, +, ?, ·, |
                while (!operator_st.empty() && precedence_of(operator_st.top().type) >= precedence_of(tok.type)) {
                    out.push_back(operator_st.top());
                    operator_st.pop();
                }
                operator_st.push(tok);
                break;
        }
    }

    while (!operator_st.empty()) {
        out.push_back(operator_st.top());
        operator_st.pop();
    }

    return out;
}

}  // namespace tokenizer
