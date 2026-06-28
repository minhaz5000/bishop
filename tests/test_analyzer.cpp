#include "bishop/analyzer.hpp"
#include "framework.hpp"

using namespace bishop::analyzer;

TEST(analyse_empty_pattern)
{
    auto info = analyse("");
    ASSERT_EQ(info.estimated_states, 0u);
    ASSERT_EQ(info.star_height, 0);
    ASSERT_EQ(info.nesting_depth, 0);
}

TEST(analyse_literal)
{
    auto info = analyse("abc");
    ASSERT_TRUE(info.estimated_states >= 2u);
    ASSERT_EQ(info.star_height, 0);
    ASSERT_EQ(info.quantifier_count, 0);
}

TEST(analyse_star_height_1)
{
    auto info = analyse("a*");
    ASSERT_EQ(info.star_height, 1);
}

TEST(analyse_star_height_nested)
{
    auto info = analyse("(a*)*");
    ASSERT_EQ(info.star_height, 2);
}

TEST(analyse_nesting_depth)
{
    auto info = analyse("((a))");
    ASSERT_EQ(info.nesting_depth, 2);
}

TEST(analyse_group_count)
{
    auto info = analyse("(a)(b)(c)");
    ASSERT_EQ(info.group_count, 3);
}

TEST(analyse_alternation_count)
{
    auto info = analyse("a|b|c");
    ASSERT_EQ(info.alternation_count, 2);
}

TEST(analyse_quantifier_count)
{
    auto info = analyse("a*b+c?");
    ASSERT_EQ(info.quantifier_count, 3);
}

TEST(analyse_uses_dot)
{
    ASSERT_TRUE(analyse("a.b").uses_dot);
    ASSERT_FALSE(analyse("abc").uses_dot);
}

TEST(analyse_uses_charclass)
{
    ASSERT_TRUE(analyse("[a-z]+").uses_charclass);
    ASSERT_FALSE(analyse("abc").uses_charclass);
}

TEST(analyse_anchor_start)
{
    ASSERT_TRUE(analyse("^abc").uses_anchor_start);
    ASSERT_FALSE(analyse("abc").uses_anchor_start);
}

TEST(analyse_anchor_end)
{
    ASSERT_TRUE(analyse("abc$").uses_anchor_end);
    ASSERT_FALSE(analyse("abc").uses_anchor_end);
}

TEST(analyse_uses_lookahead)
{
    ASSERT_TRUE(analyse("(?=abc)def").uses_lookahead);
    ASSERT_FALSE(analyse("abc").uses_lookahead);
}

TEST(analyse_complexity_simple)
{
    auto a = analyse("a");
    auto b = analyse("(a|b)*(c|d)*e");
    ASSERT_TRUE(b.complexity_score > a.complexity_score);
}

TEST(has_catastrophic_false_simple)
{
    ASSERT_FALSE(has_catastrophic_backtracking("a*"));
    ASSERT_FALSE(has_catastrophic_backtracking("abc"));
    ASSERT_FALSE(has_catastrophic_backtracking("[a-z]+"));
}

TEST(has_catastrophic_nested_star)
{
    ASSERT_TRUE(has_catastrophic_backtracking("(a*)*"));
}

TEST(estimate_state_count_grows_with_complexity)
{
    auto t1 = tokenizer::tokenize("a");
    auto t2 = tokenizer::tokenize("(a|b|c)*d");
    tokenizer::strip_anchors(t1);
    tokenizer::strip_anchors(t2);
    auto p1 = tokenizer::to_postfix(tokenizer::insert_concats(t1));
    auto p2 = tokenizer::to_postfix(tokenizer::insert_concats(t2));
    ASSERT_TRUE(estimate_state_count(p2) > estimate_state_count(p1));
}

TEST(pattern_summary_non_empty)
{
    auto info = analyse("a+b*c?");
    auto s = pattern_summary(info);
    ASSERT_FALSE(s.empty());
    ASSERT_TRUE(s.find("score=") != std::string::npos);
}

TEST(pattern_summary_catastrophic_label)
{
    auto info = analyse("(a*)*b");
    auto s = pattern_summary(info);
    ASSERT_TRUE(s.find("CATASTROPHIC") != std::string::npos);
}

TEST(analyse_repeat_quantifier)
{
    auto info = analyse("a{3,7}");
    ASSERT_EQ(info.quantifier_count, 1);
    ASSERT_TRUE(info.estimated_states >= 6u);
}
