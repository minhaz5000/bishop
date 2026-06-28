#ifndef BISHOP_LEXER_HPP_
#define BISHOP_LEXER_HPP_

#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "bishop/error.hpp"
#include "bishop/flags.hpp"
#include "bishop/regex.hpp"

namespace bishop {

class LexError : public RegexError {
public:
    LexError(const std::string& msg, size_t offset, size_t line, size_t col)
        : RegexError("lex error at line " + std::to_string(line) + ", col " + std::to_string(col) + " (offset " +
                     std::to_string(offset) + "): " + msg),
          offset_(offset),
          line_(line),
          col_(col)
    {}

    size_t offset() const noexcept
    {
        return offset_;
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
    size_t offset_, line_, col_;
};

struct LexToken {
    std::string type;
    std::string value;
    size_t line{1};
    size_t col{1};
    size_t offset{0};
    size_t length{0};

    bool empty() const noexcept
    {
        return type.empty();
    }
    bool is(const std::string& t) const noexcept
    {
        return type == t;
    }
};

using LexAction = std::function<std::string(const std::string&)>;

class Lexer {
public:
    Lexer() = default;

    void add_rule(const std::string& name, const std::string& pattern, Flag flags = Flag::NONE);

    void add_rule(const std::string& name, const std::string& pattern, LexAction action, Flag flags = Flag::NONE);

    void skip(const std::string& pattern, Flag flags = Flag::NONE);

    std::vector<LexToken> scan(const std::string& input) const;
    std::vector<LexToken> tokenize(const std::string& input) const;

    size_t rule_count() const noexcept
    {
        return rules_.size();
    }
    bool has_rule(const std::string& name) const noexcept;
    void remove_rule(const std::string& name);
    void clear()
    {
        rules_.clear();
    }

    const std::vector<LexToken>& last_tokens() const noexcept
    {
        return last_;
    }

private:
    struct Rule {
        std::string name;
        Regex re;
        LexAction action;
        bool skip{false};
        int priority{0};
    };

    size_t match_at(const Rule& rule, const std::string& input, size_t pos) const;

    LexToken make_token(const Rule& rule, const std::string& input, size_t start, size_t len, size_t line,
                        size_t col) const;

    std::vector<Rule> rules_;
    mutable std::vector<LexToken> last_;
};

}  // namespace bishop
#endif
