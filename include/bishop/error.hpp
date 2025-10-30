#ifndef BISHOP_ERROR_HPP_
#define BISHOP_ERROR_HPP_

#include <stdexcept>
#include <string>

namespace bishop {

class RegexError : public std::runtime_error {
public:
    explicit RegexError(const std::string& msg)
        : std::runtime_error(msg) {}
};

class ParseError : public RegexError {
public:
    explicit ParseError(const std::string& msg)
        : RegexError("parse error: " + msg) {}

    explicit ParseError(const std::string& msg, size_t pos)
        : RegexError("parse error at position " + std::to_string(pos) + ": " + msg)
        , pos_(pos) {}

    size_t position() const noexcept { return pos_; }

private:
    size_t pos_{std::string::npos};
};

class CompileError : public RegexError {
public:
    explicit CompileError(const std::string& msg)
        : RegexError("compile error: " + msg) {}
};

class MatchError : public RegexError {
public:
    explicit MatchError(const std::string& msg)
        : RegexError("match error: " + msg) {}
};

}  // namespace bishop
#endif
