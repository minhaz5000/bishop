#include "bishop/regex.hpp"
#include "bishop/tokenizer.hpp"
#include "framework.hpp"

using bishop::Regex;
using tokenizer::tokenize;
using tokenizer::TokenType;

TEST(tokenizer_lookahead_token)
{
    auto t = tokenize("(?=abc)");
    bool found = false;
    for (auto& tok : t)
        if (tok.type == TokenType::Lookahead) {
            found = true;
            break;
        }
    ASSERT_TRUE(found);
}

TEST(tokenizer_neg_lookahead_token)
{
    auto t = tokenize("(?!abc)");
    bool found = false;
    for (auto& tok : t)
        if (tok.type == TokenType::NegLookahead) {
            found = true;
            break;
        }
    ASSERT_TRUE(found);
}

TEST(tokenizer_non_capturing_group)
{
    auto t = tokenize("(?:ab)");
    bool found_group = false;
    for (auto& tok : t)
        if (tok.type == TokenType::Lookahead || tok.type == TokenType::NegLookahead) {
            found_group = true;
            break;
        }
    ASSERT_FALSE(found_group);
}

TEST(lookahead_token_count)
{
    auto t = tokenize("(?=x)");
    size_t cnt = 0;
    for (auto& tok : t)
        if (tok.type == TokenType::Lookahead) ++cnt;
    ASSERT_EQ(cnt, 1u);
}

TEST(neg_lookahead_token_count)
{
    auto t = tokenize("(?!x)");
    size_t cnt = 0;
    for (auto& tok : t)
        if (tok.type == TokenType::NegLookahead) ++cnt;
    ASSERT_EQ(cnt, 1u);
}

TEST(lazy_and_lookahead_together_tokenize)
{
    auto t = tokenize("a+?(?=b)");
    bool has_lazy = false, has_look = false;
    for (auto& tok : t) {
        if (tok.type == TokenType::Lazy) has_lazy = true;
        if (tok.type == TokenType::Lookahead) has_look = true;
    }
    ASSERT_TRUE(has_lazy);
    ASSERT_TRUE(has_look);
}

TEST(named_group_still_parses_after_lookahead_addition)
{
    auto t = tokenize("(?P<name>abc)");
    bool found = false;
    for (auto& tok : t)
        if (tok.type == TokenType::NamedGroup) {
            found = true;
            break;
        }
    ASSERT_TRUE(found);
}

TEST(lookahead_compile_does_not_throw)
{
    ASSERT_NO_THROW(Regex r("(?=abc)def"));
}

TEST(neg_lookahead_compile_does_not_throw)
{
    ASSERT_NO_THROW(Regex r("(?!abc)def"));
}

TEST(lookahead_combined_with_charclass)
{
    ASSERT_NO_THROW(Regex r("(?=[[:alpha:]])\\w+"));
}
