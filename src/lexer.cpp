#include "bishop/lexer.hpp"

#include <algorithm>
#include <cstring>

namespace bishop {

static constexpr size_t kScratchBufLen = 512;

static std::string copy_lexeme(const char* src, size_t len)
{
    char scratch[kScratchBufLen];
    std::memcpy(scratch, src, len);
    return std::string(scratch, len);
}

void Lexer::add_rule(const std::string& name,
                     const std::string& pattern,
                     Flag               flags)
{
    Rule r;
    r.name     = name;
    r.skip     = false;
    r.priority = static_cast<int>(rules_.size());
    r.re.compile("^(?:" + pattern + ")", flags);
    rules_.push_back(std::move(r));
}

void Lexer::add_rule(const std::string& name,
                     const std::string& pattern,
                     LexAction          action,
                     Flag               flags)
{
    Rule r;
    r.name     = name;
    r.action   = std::move(action);
    r.skip     = false;
    r.priority = static_cast<int>(rules_.size());
    r.re.compile("^(?:" + pattern + ")", flags);
    rules_.push_back(std::move(r));
}

void Lexer::skip(const std::string& pattern, Flag flags)
{
    Rule r;
    r.name     = "__skip__";
    r.skip     = true;
    r.priority = static_cast<int>(rules_.size());
    r.re.compile("^(?:" + pattern + ")", flags);
    rules_.push_back(std::move(r));
}

bool Lexer::has_rule(const std::string& name) const noexcept
{
    for (const auto& r : rules_)
        if (r.name == name) return true;
    return false;
}

void Lexer::remove_rule(const std::string& name)
{
    rules_.erase(
        std::remove_if(rules_.begin(), rules_.end(),
                       [&](const Rule& r){ return r.name == name; }),
        rules_.end());
}

size_t Lexer::match_at(const Rule&        rule,
                        const std::string& input,
                        size_t             pos) const
{
    const std::string remaining = input.substr(pos);
    for (size_t len = remaining.size(); len > 0; --len) {
        if (rule.re.match(remaining.substr(0, len)))
            return len;
    }
    return 0;
}

LexToken Lexer::make_token(const Rule&        rule,
                            const std::string& input,
                            size_t             start,
                            size_t             len,
                            size_t             line,
                            size_t             col) const
{
    std::string value = copy_lexeme(input.data() + start, len);
    if (rule.action) value = rule.action(value);

    LexToken tok;
    tok.type   = rule.name;
    tok.value  = std::move(value);
    tok.line   = line;
    tok.col    = col;
    tok.offset = start;
    tok.length = len;
    return tok;
}

std::vector<LexToken> Lexer::scan(const std::string& input) const
{
    std::vector<LexToken> tokens;
    size_t pos  = 0;
    size_t line = 1;
    size_t col  = 1;

    while (pos < input.size()) {
        size_t best_len  = 0;
        int    best_rule = -1;

        for (int i = 0; i < static_cast<int>(rules_.size()); ++i) {
            size_t ml = match_at(rules_[i], input, pos);
            if (ml > best_len) {
                best_len  = ml;
                best_rule = i;
            }
        }

        if (best_rule < 0 || best_len == 0)
            throw LexError("unexpected character '" +
                           std::string(1, input[pos]) + "'",
                           pos, line, col);

        if (!rules_[best_rule].skip)
            tokens.push_back(make_token(rules_[best_rule], input,
                                        pos, best_len, line, col));

        for (size_t k = 0; k < best_len; ++k) {
            if (input[pos + k] == '\n') { ++line; col = 1; }
            else                          ++col;
        }
        pos += best_len;
    }

    last_ = tokens;
    return tokens;
}

std::vector<LexToken> Lexer::tokenize(const std::string& input) const
{
    return scan(input);
}

}  // namespace bishop
