#include "tokenizer.hpp"

#include <cctype>
#include <stack>

namespace tokenizer {

std::vector<Token> tokenize(const std::string& s)
{
    if (s.empty()) return {};

    std::vector<Token> tokens;
    tokens.reserve(s.size());

    auto it = s.cbegin();
    while (it != s.cend()) {
        char ch = static_cast<unsigned char>(*it);

        if (std::isalnum(ch)) {
            tokens.push_back({TokenType::Literal, ch});
        } else {
            switch (ch) {
                case '+': tokens.push_back({TokenType::Plus,     ch}); break;
                case '*': tokens.push_back({TokenType::Star,     ch}); break;
                case '?': tokens.push_back({TokenType::Question, ch}); break;
                case '|': tokens.push_back({TokenType::Union,    ch}); break;
                case '(': tokens.push_back({TokenType::LParen,   ch}); break;
                case ')': tokens.push_back({TokenType::RParen,   ch}); break;
                case '.': tokens.push_back({TokenType::Dot,      ch}); break;
                case '\\':
                    ++it;
                    tokens.push_back({TokenType::Literal, static_cast<unsigned char>(*it)});
                    break;
                default:  tokens.push_back({TokenType::Unknown,  ch}); break;
            }
        }
        ++it;
    }
    return tokens;
}

std::vector<Token> insert_concats(const std::vector<Token>& s)
{
    if (s.empty()) return {};

    std::vector<Token> tokens;
    tokens.reserve(s.size() * 2);

    for (size_t i = 0; i < s.size(); ++i) {
        tokens.push_back(s[i]);
        if (i + 1 >= s.size()) break;

        const auto& curr = s[i];
        const auto& next = s[i + 1];

        bool curr_ok = curr.type == TokenType::Literal  ||
                       curr.type == TokenType::RParen   ||
                       curr.type == TokenType::Star     ||
                       curr.type == TokenType::Plus     ||
                       curr.type == TokenType::Question ||
                       curr.type == TokenType::Dot;

        bool next_ok = next.type == TokenType::Literal ||
                       next.type == TokenType::LParen  ||
                       next.type == TokenType::Dot;

        if (curr_ok && next_ok)
            tokens.push_back({TokenType::Concat, '.'});
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

    std::stack<Token> op_st;
    std::vector<Token> out;
    out.reserve(s.size() * 2);

    for (const auto& tok : s) {
        switch (tok.type) {
            case TokenType::Literal:
            case TokenType::Dot:
                out.push_back(tok);
                break;
            case TokenType::LParen:
                op_st.push(tok);
                break;
            case TokenType::RParen:
                while (!op_st.empty() && op_st.top().type != TokenType::LParen) {
                    out.push_back(op_st.top());
                    op_st.pop();
                }
                if (!op_st.empty()) op_st.pop();
                break;
            default:
                while (!op_st.empty() &&
                       precedence_of(op_st.top().type) >= precedence_of(tok.type)) {
                    out.push_back(op_st.top());
                    op_st.pop();
                }
                op_st.push(tok);
                break;
        }
    }

    while (!op_st.empty()) {
        out.push_back(op_st.top());
        op_st.pop();
    }

    return out;
}

}  // namespace tokenizer
