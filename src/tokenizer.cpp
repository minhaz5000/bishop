#include "bishop/error.hpp"
#include "bishop/tokenizer.hpp"
#include "bishop/flags.hpp"
#include "bishop/posix.hpp"

#include <cctype>
#include <stack>

namespace tokenizer {

static std::vector<bool> cs_range(unsigned char lo, unsigned char hi) {
    std::vector<bool> cs(256, false);
    for (unsigned char c = lo; c <= hi; ++c) cs[c] = true;
    return cs;
}

static std::vector<bool> cs_set(std::initializer_list<unsigned char> chars) {
    std::vector<bool> cs(256, false);
    for (unsigned char c : chars) cs[c] = true;
    return cs;
}

static std::vector<bool> cs_union(std::vector<bool> a, const std::vector<bool>& b) {
    for (size_t i = 0; i < 256; ++i) a[i] = a[i] || b[i];
    return a;
}

static std::vector<bool> cs_negate(std::vector<bool> cs) {
    for (auto& b : cs) b = !b;
    return cs;
}

static Token make_charclass(std::vector<bool> cs) {
    Token tok;
    tok.type     = TokenType::CharClass;
    tok.value    = 0;
    tok.char_set = std::move(cs);
    return tok;
}

static std::vector<bool> cs_word() {
    return cs_union(cs_union(cs_range('a','z'), cs_range('A','Z')),
                    cs_union(cs_range('0','9'), cs_set({'_'})));
}

static std::vector<bool> cs_space() {
    return cs_set({' ', '\t', '\n', '\r', '\f', '\v'});
}

std::vector<Token> tokenize(const std::string& s, bishop::Flag flags)
{
    if (s.empty()) return {};

    std::vector<Token> tokens;
    tokens.reserve(s.size());

    bool extended = bishop::has_flag(flags, bishop::Flag::EXTENDED);
    auto it = s.cbegin();
    while (it != s.cend()) {
        unsigned char ch = static_cast<unsigned char>(*it);
        if (extended) {
            if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') { ++it; continue; }
            if (ch == '#') {
                while (it != s.cend() && *it != '\n') ++it;
                continue;
            }
        }

        if (std::isalnum(ch)) {
            tokens.push_back({TokenType::Literal, (char)ch, {}, 0, -1});
        } else {
            switch (ch) {
                case +: {
                    tokens.push_back({TokenType::Plus, '+', {}, 0, -1});
                    if ((it + 1) != s.cend() && *(it + 1) == '?') {
                        ++it;
                        tokens.push_back({TokenType::Lazy, '?', {}, 0, -1});
                    }
                    break;
                }
                case *: {
                    tokens.push_back({TokenType::Star, '*', {}, 0, -1});
                    if ((it + 1) != s.cend() && *(it + 1) == '?') {
                        ++it;
                        tokens.push_back({TokenType::Lazy, '?', {}, 0, -1});
                    }
                    break;
                }
                case '?': tokens.push_back({TokenType::Question, '?', {}, 0, -1}); break;
                case '|': tokens.push_back({TokenType::Union,    '|', {}, 0, -1}); break;
                case '(': {
                    auto peek = it + 1;
                    if (peek != s.cend() && *peek == '?') {
                        auto peek2 = peek + 1;
                        if (peek2 != s.cend() && *peek2 == ':') {
                            tokens.push_back({TokenType::LParen, '(', {}, 0, -1});
                            it = peek2;
                        } else if (peek2 != s.cend() && *peek2 == '=') {
                            tokens.push_back({TokenType::Lookahead,    '(', {}, 0, -1});
                            it = peek2;
                        } else if (peek2 != s.cend() && *peek2 == '!') {
                            tokens.push_back({TokenType::NegLookahead, '(', {}, 0, -1});
                            it = peek2;
                        } else if (peek2 != s.cend() && *peek2 == 'P') {
                            auto peek3 = peek2 + 1;
                            if (peek3 != s.cend() && *peek3 == '<') {
                                ++peek3;
                                std::string gname;
                                while (peek3 != s.cend() && *peek3 != '>')
                                    { gname += *peek3; ++peek3; }
                                Token tok;
                                tok.type       = TokenType::NamedGroup;
                                tok.value      = '(';
                                tok.group.name = gname;
                                tokens.push_back(std::move(tok));
                                it = peek3;
                            } else {
                                tokens.push_back({TokenType::LParen, '(', {}, 0, -1});
                            }
                        } else {
                            tokens.push_back({TokenType::LParen, '(', {}, 0, -1});
                        }
                    } else {
                        Token tok;
                        tok.type        = TokenType::GroupStart;
                        tok.value       = '(';
                        tokens.push_back(std::move(tok));
                    }
                    break;
                }
                case ')': tokens.push_back({TokenType::RParen,   ')', {}, 0, -1}); break;
                case '.': tokens.push_back({TokenType::Dot,         '.', {}, 0, -1}); break;
                case '^': tokens.push_back({TokenType::AnchorStart, '^', {}, 0, -1}); break;
                case '$': tokens.push_back({TokenType::AnchorEnd,   '$', {}, 0, -1}); break;

                case '\\': {
                    ++it;
                    unsigned char esc = static_cast<unsigned char>(*it);
                    switch (esc) {
                        case 'D':
                            tokens.push_back(make_charclass(cs_negate(cs_range('0', '9'))));
                            break;
                        case 'W':
                            tokens.push_back(make_charclass(cs_negate(cs_word())));
                            break;
                        case 'S':
                            tokens.push_back(make_charclass(cs_negate(cs_space())));
                            break;
                        case 'd':
                            tokens.push_back(make_charclass(cs_range('0', '9')));
                            break;
                        case 'w':
                            tokens.push_back(make_charclass(cs_word()));
                            break;
                        case 's':
                            tokens.push_back(make_charclass(cs_space()));
                            break;
                        case 'b': tokens.push_back({TokenType::WordBoundary,    'b', {}, 0, -1}); break;
                        case 'B': tokens.push_back({TokenType::NonWordBoundary, 'B', {}, 0, -1}); break;
                        case 'n': tokens.push_back({TokenType::Literal, '\n', {}, 0, -1}); break;
                        case 't': tokens.push_back({TokenType::Literal, '\t', {}, 0, -1}); break;
                        case 'r': tokens.push_back({TokenType::Literal, '\r', {}, 0, -1}); break;
                        case 'v': tokens.push_back({TokenType::Literal, '\v', {}, 0, -1}); break;
                        case '0': tokens.push_back({TokenType::Literal, '\0', {}, 0, -1}); break;
                        default:
                            tokens.push_back({TokenType::Literal, static_cast<char>(esc), {}, 0, -1});
                    }
                    break;
                }

                case '[': {
                    ++it;
                    bool negated = false;
                    if (it != s.cend() && *it == '^') { negated = true; ++it; }

                    bool in_class[256] = {};
                    while (it != s.cend() && *it != ']') {
                        if (*it == '[' && (it + 1) != s.cend() && *(it + 1) == ':') {
                            it += 2;
                            std::string pname;
                            while (it != s.cend() && *it != ':') { pname += *it; ++it; }
                            if (it != s.cend()) ++it;
                            if (it != s.cend() && *it == ']') ++it;
                            auto pcls = bishop::posix::from_name(pname);
                            for (int pi = 0; pi < 256; ++pi)
                                if (pcls[pi]) in_class[pi] = true;
                            continue;
                        }
                        unsigned char c = static_cast<unsigned char>(*it);
                        ++it;
                        if (it != s.cend() && *it == '-' &&
                            (it + 1) != s.cend() && *(it + 1) != ']') {
                            ++it;
                            unsigned char ec = static_cast<unsigned char>(*it);
                            ++it;
                            for (unsigned char r = c; r <= ec; ++r) in_class[r] = true;
                        } else {
                            in_class[c] = true;
                        }
                    }
                    if (it != s.cend()) ++it;

                    Token tok;
                    tok.type = TokenType::CharClass;
                    tok.value = 0;
                    tok.char_set.resize(256, false);
                    for (int i = 0; i < 256; ++i)
                        tok.char_set[i] = negated ? !in_class[i] : (bool)in_class[i];
                    tokens.push_back(std::move(tok));
                    break;
                }

                case '{': {
                    ++it;
                    int mn = 0, mx = -1;
                    while (it != s.cend() && std::isdigit(static_cast<unsigned char>(*it)))
                        { mn = mn * 10 + (*it - '0'); ++it; }
                    if (it != s.cend() && *it == ',') {
                        ++it;
                        if (it != s.cend() && std::isdigit(static_cast<unsigned char>(*it))) {
                            mx = 0;
                            while (it != s.cend() && std::isdigit(static_cast<unsigned char>(*it)))
                                { mx = mx * 10 + (*it - '0'); ++it; }
                        }
                    } else { mx = mn; }
                    Token tok;
                    tok.type = TokenType::Repeat; tok.value = '{';
                    tok.repeat_min = mn; tok.repeat_max = mx;
                    tokens.push_back(std::move(tok));
                    if (it != s.cend() && *it == '?') {
                        ++it;
                        tokens.push_back({TokenType::Lazy, '?', {}, 0, -1});
                    }
                    break;
                }

                default: tokens.push_back({TokenType::Unknown, (char)ch, {}, 0, -1}); break;
            }
        }
        ++it;
    }
    return tokens;
}

static void apply_ignorecase(std::vector<Token>& tokens) {
    for (auto& tok : tokens) {
        if (tok.type == TokenType::Literal) {
            unsigned char lo = static_cast<unsigned char>(std::tolower((unsigned char)tok.value));
            unsigned char up = static_cast<unsigned char>(std::toupper((unsigned char)tok.value));
            if (lo != up) {
                tok.char_set.assign(256, false);
                tok.char_set[lo] = true;
                tok.char_set[up] = true;
                tok.type  = TokenType::CharClass;
                tok.value = 0;
            }
        } else if (tok.type == TokenType::CharClass) {
            std::vector<bool> extra(256, false);
            for (int i = 0; i < 256; ++i) {
                if (!tok.char_set[i]) continue;
                extra[std::tolower(i)] = true;
                extra[std::toupper(i)] = true;
            }
            tok.char_set = extra;
        }
    }
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
        bool curr_ok = curr.type == TokenType::Literal  || curr.type == TokenType::RParen   ||
                       curr.type == TokenType::Star     || curr.type == TokenType::Plus      ||
                       curr.type == TokenType::Question || curr.type == TokenType::Repeat    ||
                       curr.type == TokenType::Dot      || curr.type == TokenType::CharClass;
        bool next_ok = next.type == TokenType::Literal  || next.type == TokenType::LParen   ||
                       next.type == TokenType::Dot      || next.type == TokenType::CharClass;
        if (curr_ok && next_ok)
            tokens.push_back({TokenType::Concat, '.', {}, 0, -1});
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
            case TokenType::CharClass:
                out.push_back(tok); break;
            case TokenType::LParen:
                op_st.push(tok); break;
            case TokenType::RParen:
                while (!op_st.empty() && op_st.top().type != TokenType::LParen)
                    { out.push_back(op_st.top()); op_st.pop(); }
                if (!op_st.empty()) op_st.pop();
                break;
            default:
                while (!op_st.empty() &&
                       precedence_of(op_st.top().type) >= precedence_of(tok.type))
                    { out.push_back(op_st.top()); op_st.pop(); }
                op_st.push(tok);
                break;
        }
    }
    while (!op_st.empty()) { out.push_back(op_st.top()); op_st.pop(); }
    return out;
}

}  // namespace tokenizer
