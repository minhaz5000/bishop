#include "bishop/posix.hpp"
#include "bishop/regex.hpp"
#include "bishop/tokenizer.hpp"
#include "framework.hpp"

using bishop::Regex;
using tokenizer::tokenize;
using tokenizer::TokenType;

TEST(tokenizer_word_boundary_token)
{
    auto t = tokenize("\\b");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_EQ(t[0].type, TokenType::WordBoundary);
}

TEST(tokenizer_non_word_boundary_token)
{
    auto t = tokenize("\\B");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_EQ(t[0].type, TokenType::NonWordBoundary);
}

TEST(posix_cntrl)
{
    auto cs = bishop::posix::cls_cntrl();
    ASSERT_TRUE(cs[0]);
    ASSERT_TRUE(cs['\t']);
    ASSERT_FALSE(cs['a']);
}

TEST(posix_graph)
{
    auto cs = bishop::posix::cls_graph();
    ASSERT_TRUE(cs['a']);
    ASSERT_TRUE(cs['!']);
    ASSERT_FALSE(cs[' ']);
    ASSERT_FALSE(cs['\n']);
}

TEST(posix_print)
{
    auto cs = bishop::posix::cls_print();
    ASSERT_TRUE(cs['a']);
    ASSERT_TRUE(cs[' ']);
    ASSERT_FALSE(cs['\n']);
}

TEST(posix_cls_blank)
{
    auto cs = bishop::posix::cls_blank();
    ASSERT_TRUE(cs[' ']);
    ASSERT_TRUE(cs['\t']);
    ASSERT_FALSE(cs['a']);
    ASSERT_FALSE(cs['\n']);
}

TEST(posix_nested_in_charclass)
{
    Regex r("[[:upper:][:digit:]]+");
    ASSERT_TRUE(r.match("ABC123"));
    ASSERT_FALSE(r.match("abc"));
}

TEST(charclass_with_posix_and_range)
{
    Regex r("[[:digit:]a-f]+");
    ASSERT_TRUE(r.match("1234abc"));
    ASSERT_FALSE(r.match("xyz"));
}

TEST(posix_alnum_via_tokenizer)
{
    auto t = tokenize("[[:alnum:]]");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_EQ(t[0].type, TokenType::CharClass);
    ASSERT_TRUE(t[0].char_set['a']);
    ASSERT_TRUE(t[0].char_set['5']);
    ASSERT_FALSE(t[0].char_set['!']);
}
