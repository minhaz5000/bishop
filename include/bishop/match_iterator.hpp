#ifndef BISHOP_MATCH_ITERATOR_HPP_
#define BISHOP_MATCH_ITERATOR_HPP_

#include <iterator>
#include <string>
#include "bishop/regex.hpp"
#include "bishop/match_result.hpp"

namespace bishop {

class MatchIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = MatchResult;
    using difference_type   = std::ptrdiff_t;
    using pointer           = const MatchResult*;
    using reference         = const MatchResult&;

    MatchIterator() = default;

    MatchIterator(Regex* re, const std::string& text)
        : re_(re), text_(&text), pos_(0) {
        advance();
    }

    reference operator*() const { return current_; }
    pointer operator->() const { return &current_; }

    MatchIterator& operator++() {
        advance();
        return *this;
    }

    MatchIterator operator++(int) {
        MatchIterator tmp = *this;
        advance();
        return tmp;
    }

    bool operator==(const MatchIterator& other) const {
        if (!matched_ && !other.matched_) return true;
        return matched_ == other.matched_ &&
               pos_ == other.pos_ &&
               text_ == other.text_;
    }

    bool operator!=(const MatchIterator& other) const {
        return !(*this == other);
    }

private:
    void advance() {
        if (!re_ || !text_ || pos_ > text_->size()) {
            matched_ = false;
            return;
        }

        auto [s, e] = re_->search(text_->substr(pos_));
        if (s == std::string::npos) {
            matched_ = false;
            pos_     = std::string::npos;
            return;
        }

        current_ = MatchResult(*text_, pos_ + s, pos_ + e);
        matched_ = true;
        pos_     += (e == s) ? e + 1 : e;
    }

    Regex*             re_{nullptr};
    const std::string* text_{nullptr};
    size_t             pos_{0};
    bool               matched_{false};
    MatchResult        current_;
};

}
#endif
