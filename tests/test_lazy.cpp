#include "framework.hpp"
#include "bishop/regex.hpp"
#include "bishop/tokenizer.hpp"

using bishop::Regex;
using tokenizer::tokenize;
using tokenizer::TokenType;

TEST(tokenizer_lazy_after_star) {
    auto t = tokenize("a*?");
    ASSERT_EQ(t.size(), 3u);
    ASSERT_EQ(t[1].type, TokenType::Star);
    ASSERT_EQ(t[2].type, TokenType::Lazy);
}

TEST(tokenizer_lazy_after_plus) {
    auto t = tokenize("a+?");
    ASSERT_EQ(t[1].type, TokenType::Plus);
    ASSERT_EQ(t[2].type, TokenType::Lazy);
}

TEST(tokenizer_lazy_after_question) {
    auto t = tokenize("a??");
    ASSERT_EQ(t[1].type, TokenType::Question);
    ASSERT_EQ(t[2].type, TokenType::Lazy);
}

TEST(tokenizer_lazy_after_repeat) {
    auto t = tokenize("a{2,4}?");
    bool found_lazy = false;
    for (auto& tok : t)
        if (tok.type == TokenType::Lazy) { found_lazy = true; break; }
    ASSERT_TRUE(found_lazy);
}

TEST(tokenizer_no_lazy_without_question) {
    auto t = tokenize("a*");
    for (auto& tok : t)
        ASSERT_NE(tok.type, TokenType::Lazy);
}

TEST(lazy_star_matches_empty) {
    Regex r("a*?");
    ASSERT_TRUE(r.match(""));
}

TEST(lazy_star_matches_one) {
    Regex r("a*?");
    ASSERT_TRUE(r.match("a"));
}

TEST(lazy_plus_requires_one) {
    Regex r("a+?");
    ASSERT_FALSE(r.match(""));
    ASSERT_TRUE(r.match("a"));
}

TEST(lazy_question_matches_empty) {
    Regex r("a??");
    ASSERT_TRUE(r.match(""));
}

TEST(lazy_question_matches_one) {
    Regex r("a??");
    ASSERT_TRUE(r.match("a"));
}

TEST(greedy_star_no_lazy_token) {
    auto t = tokenize("a*b");
    for (auto& tok : t)
        ASSERT_NE(tok.type, TokenType::Lazy);
}

TEST(lazy_in_group) {
    auto t = tokenize("(ab)+?");
    bool found_lazy = false;
    for (auto& tok : t)
        if (tok.type == TokenType::Lazy) { found_lazy = true; break; }
    ASSERT_TRUE(found_lazy);
}
