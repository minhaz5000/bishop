#include "bishop/token_stream.hpp"

#include <sstream>
#include <stdexcept>

namespace bishop {

TokenStream::TokenStream(std::vector<LexToken> tokens) : tokens_(std::move(tokens)), pos_(0) {}

TokenStream::TokenStream(Lexer& lexer, const std::string& input) : tokens_(lexer.scan(input)), pos_(0) {}

bool TokenStream::eof() const noexcept
{
    return pos_ >= tokens_.size();
}

size_t TokenStream::remaining() const noexcept
{
    return pos_ < tokens_.size() ? tokens_.size() - pos_ : 0;
}

const LexToken& TokenStream::peek() const
{
    if (eof()) throw TokenStreamError("unexpected end of token stream");
    return tokens_[pos_];
}

const LexToken* TokenStream::peek_at(size_t offset) const noexcept
{
    size_t idx = pos_ + offset;
    return idx < tokens_.size() ? &tokens_[idx] : nullptr;
}

bool TokenStream::peek_is(const std::string& type) const noexcept
{
    return !eof() && tokens_[pos_].type == type;
}

LexToken TokenStream::consume()
{
    if (eof()) throw TokenStreamError("unexpected end of token stream");
    return tokens_[pos_++];
}

LexToken TokenStream::expect(const std::string& type)
{
    if (eof()) {
        throw TokenStreamError("expected '" + type + "' but got end of stream", "<eof>", 0, 0);
    }

    const LexToken& t = tokens_[pos_];
    if (t.type != type) {
        throw TokenStreamError("expected '" + type + "'", t.type, t.line, t.col);
    }

    return tokens_[pos_++];
}

std::optional<LexToken> TokenStream::try_consume(const std::string& type)
{
    if (!eof() && tokens_[pos_].type == type) return tokens_[pos_++];
    return std::nullopt;
}

void TokenStream::skip_while(const std::function<bool(const LexToken&)>& pred)
{
    while (!eof() && pred(tokens_[pos_])) ++pos_;
}

void TokenStream::skip(size_t n)
{
    pos_ += n;
    if (pos_ > tokens_.size()) pos_ = tokens_.size();
}

void TokenStream::skip_all()
{
    pos_ = tokens_.size();
}

void TokenStream::expect_eof() const
{
    if (!eof()) {
        const LexToken& t = tokens_[pos_];
        throw TokenStreamError("expected end of stream but got '" + t.type + "'", t.type, t.line, t.col);
    }
}

void TokenStream::save()
{
    checkpoints_.push_back(pos_);
}

void TokenStream::restore()
{
    if (checkpoints_.empty()) throw TokenStreamError("restore() called with no saved checkpoint");
    pos_ = checkpoints_.back();
    checkpoints_.pop_back();
}

void TokenStream::commit()
{
    if (checkpoints_.empty()) throw TokenStreamError("commit() called with no saved checkpoint");
    checkpoints_.pop_back();
}

std::vector<LexToken> TokenStream::collect_until(const std::string& end_type)
{
    std::vector<LexToken> result;
    while (!eof() && tokens_[pos_].type != end_type) result.push_back(tokens_[pos_++]);
    return result;
}

std::vector<LexToken> TokenStream::collect_while(const std::function<bool(const LexToken&)>& pred)
{
    std::vector<LexToken> result;
    while (!eof() && pred(tokens_[pos_])) result.push_back(tokens_[pos_++]);
    return result;
}

std::vector<LexToken> TokenStream::slice(size_t from, size_t to) const
{
    if (from >= tokens_.size()) return {};
    if (to > tokens_.size()) to = tokens_.size();
    return std::vector<LexToken>(tokens_.begin() + static_cast<std::ptrdiff_t>(from),
                                 tokens_.begin() + static_cast<std::ptrdiff_t>(to));
}

}  // namespace bishop
