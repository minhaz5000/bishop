#ifndef BISHOP_TOKEN_STREAM_HPP_
#define BISHOP_TOKEN_STREAM_HPP_

#include <cassert>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "bishop/lexer.hpp"

namespace bishop {

class TokenStreamError : public RegexError {
public:
    explicit TokenStreamError(const std::string& msg) : RegexError("token stream error: " + msg) {}

    TokenStreamError(const std::string& msg, const std::string& token_type, size_t line, size_t col)
        : RegexError("token stream error at line " + std::to_string(line) + ", col " + std::to_string(col) + ": " +
                     msg + " (got '" + token_type + "')"),
          token_type_(token_type),
          line_(line),
          col_(col)
    {}

    const std::string& token_type() const noexcept
    {
        return token_type_;
    }
    size_t line() const noexcept
    {
        return line_;
    }
    size_t col() const noexcept
    {
        return col_;
    }

private:
    std::string token_type_;
    size_t line_{0};
    size_t col_{0};
};

class TokenStream {
public:
    TokenStream() = default;

    explicit TokenStream(std::vector<LexToken> tokens);

    TokenStream(Lexer& lexer, const std::string& input);

    bool eof() const noexcept;
    size_t pos() const noexcept
    {
        return pos_;
    }
    size_t size() const noexcept
    {
        return tokens_.size();
    }
    size_t remaining() const noexcept;

    const LexToken& peek() const;

    const LexToken* peek_at(size_t offset) const noexcept;

    bool peek_is(const std::string& type) const noexcept;

    LexToken consume();

    LexToken expect(const std::string& type);

    std::optional<LexToken> try_consume(const std::string& type);

    void skip_while(const std::function<bool(const LexToken&)>& pred);

    void skip(size_t n = 1);

    void skip_all();

    void expect_eof() const;

    void save();

    void restore();

    void commit();

    std::vector<LexToken> collect_until(const std::string& end_type);

    std::vector<LexToken> collect_while(const std::function<bool(const LexToken&)>& pred);

    const std::vector<LexToken>& tokens() const noexcept
    {
        return tokens_;
    }

    std::vector<LexToken> slice(size_t from, size_t to) const;

private:
    std::vector<LexToken> tokens_;
    size_t pos_{0};
    std::vector<size_t> checkpoints_;
};

}  // namespace bishop
#endif  // BISHOP_TOKEN_STREAM_HPP_
