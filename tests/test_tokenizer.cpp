#include "framework.hpp"
#include "bishop/tokenizer.hpp"

using namespace tokenizer;

TEST(empty_pattern_yields_no_tokens) {
    ASSERT_TRUE(tokenize("").empty());
}

TEST(single_literal) {
    auto t = tokenize("a");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_EQ(t[0].type, TokenType::Literal);
    ASSERT_EQ(t[0].value, 'a');
}

TEST(digit_literal) {
    auto t = tokenize("7");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_EQ(t[0].type, TokenType::Literal);
    ASSERT_EQ(t[0].value, '7');
}

TEST(star_operator) {
    auto t = tokenize("a*");
    ASSERT_EQ(t.size(), 2u);
    ASSERT_EQ(t[1].type, TokenType::Star);
}

TEST(plus_operator) {
    auto t = tokenize("b+");
    ASSERT_EQ(t[1].type, TokenType::Plus);
}

TEST(question_operator) {
    auto t = tokenize("c?");
    ASSERT_EQ(t[1].type, TokenType::Question);
}

TEST(union_operator) {
    auto t = tokenize("a|b");
    ASSERT_EQ(t.size(), 3u);
    ASSERT_EQ(t[1].type, TokenType::Union);
}

TEST(parens) {
    auto t = tokenize("(a)");
    ASSERT_EQ(t.size(), 3u);
    ASSERT_EQ(t[0].type, TokenType::LParen);
    ASSERT_EQ(t[2].type, TokenType::RParen);
}

TEST(dot_wildcard) {
    auto t = tokenize(".");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_EQ(t[0].type, TokenType::Dot);
}

TEST(anchor_start) {
    auto t = tokenize("^a");
    ASSERT_EQ(t[0].type, TokenType::AnchorStart);
}

TEST(anchor_end) {
    auto t = tokenize("a$");
    ASSERT_EQ(t.back().type, TokenType::AnchorEnd);
}

TEST(charclass_basic) {
    auto t = tokenize("[abc]");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_EQ(t[0].type, TokenType::CharClass);
    ASSERT_TRUE(t[0].char_set['a']);
    ASSERT_TRUE(t[0].char_set['b']);
    ASSERT_TRUE(t[0].char_set['c']);
    ASSERT_FALSE(t[0].char_set['d']);
}

TEST(charclass_range) {
    auto t = tokenize("[a-z]");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_TRUE(t[0].char_set['a']);
    ASSERT_TRUE(t[0].char_set['m']);
    ASSERT_TRUE(t[0].char_set['z']);
    ASSERT_FALSE(t[0].char_set['A']);
}

TEST(charclass_negated) {
    auto t = tokenize("[^abc]");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_FALSE(t[0].char_set['a']);
    ASSERT_FALSE(t[0].char_set['b']);
    ASSERT_TRUE(t[0].char_set['d']);
}

TEST(escape_digit_shorthand) {
    auto t = tokenize("\\d");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_EQ(t[0].type, TokenType::CharClass);
    ASSERT_TRUE(t[0].char_set['5']);
    ASSERT_FALSE(t[0].char_set['a']);
}

TEST(escape_word_shorthand) {
    auto t = tokenize("\\w");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_TRUE(t[0].char_set['a']);
    ASSERT_TRUE(t[0].char_set['Z']);
    ASSERT_TRUE(t[0].char_set['0']);
    ASSERT_TRUE(t[0].char_set['_']);
    ASSERT_FALSE(t[0].char_set[' ']);
}

TEST(escape_space_shorthand) {
    auto t = tokenize("\\s");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_TRUE(t[0].char_set[' ']);
    ASSERT_TRUE(t[0].char_set['\t']);
    ASSERT_FALSE(t[0].char_set['a']);
}

TEST(escape_control_n) {
    auto t = tokenize("\\n");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_EQ(t[0].type, TokenType::Literal);
    ASSERT_EQ(t[0].value, '\n');
}

TEST(escape_control_t) {
    auto t = tokenize("\\t");
    ASSERT_EQ(t[0].value, '\t');
}

TEST(repeat_exact) {
    auto t = tokenize("a{3}");
    ASSERT_EQ(t.size(), 2u);
    ASSERT_EQ(t[1].type, TokenType::Repeat);
    ASSERT_EQ(t[1].repeat_min, 3);
    ASSERT_EQ(t[1].repeat_max, 3);
}

TEST(repeat_range) {
    auto t = tokenize("a{2,5}");
    ASSERT_EQ(t[1].repeat_min, 2);
    ASSERT_EQ(t[1].repeat_max, 5);
}

TEST(repeat_unbounded) {
    auto t = tokenize("a{2,}");
    ASSERT_EQ(t[1].repeat_min, 2);
    ASSERT_EQ(t[1].repeat_max, -1);
}

TEST(insert_concats_simple) {
    auto t = insert_concats(tokenize("ab"));
    ASSERT_EQ(t.size(), 3u);
    ASSERT_EQ(t[1].type, TokenType::Concat);
}

TEST(insert_concats_no_concat_before_union) {
    auto t = insert_concats(tokenize("a|b"));
    for (auto& tok : t)
        ASSERT_NE(tok.type, TokenType::Concat);
}

TEST(to_postfix_simple_concat) {
    auto post = to_postfix(insert_concats(tokenize("ab")));
    ASSERT_EQ(post.size(), 3u);
    ASSERT_EQ(post[0].type, TokenType::Literal);
    ASSERT_EQ(post[1].type, TokenType::Literal);
    ASSERT_EQ(post[2].type, TokenType::Concat);
}

TEST(to_postfix_star) {
    auto post = to_postfix(insert_concats(tokenize("a*")));
    ASSERT_EQ(post.back().type, TokenType::Star);
}

TEST(strip_anchors_removes_caret) {
    auto toks = tokenize("^abc");
    auto info  = strip_anchors(toks);
    ASSERT_TRUE(info.start);
    ASSERT_FALSE(info.end);
    ASSERT_EQ(toks.front().type, TokenType::Literal);
}

TEST(strip_anchors_removes_dollar) {
    auto toks = tokenize("abc$");
    auto info  = strip_anchors(toks);
    ASSERT_FALSE(info.start);
    ASSERT_TRUE(info.end);
    ASSERT_EQ(toks.back().type, TokenType::Literal);
}
