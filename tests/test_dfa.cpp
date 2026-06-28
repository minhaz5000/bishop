#include "bishop/dfa.hpp"
#include "bishop/nfa.hpp"
#include "bishop/tokenizer.hpp"
#include "framework.hpp"

using namespace tokenizer;

static dfa::DFA compile(const std::string& pattern)
{
    auto tokens = tokenize(pattern);
    strip_anchors(tokens);
    tokens = insert_concats(tokens);
    auto postfix = to_postfix(tokens);
    auto nfa = nfa::parse_postfix(postfix);
    return dfa::nfa_to_dfa(nfa);
}

static bool DM(const std::string& pat, const std::string& text)
{
    auto d = compile(pat);
    return dfa::dfa_match(d, text);
}

static bool MM(const std::string& pat, const std::string& text)
{
    auto d = dfa::minimize(compile(pat));
    return dfa::dfa_match(d, text);
}

TEST(dfa_literal)
{
    ASSERT_TRUE(DM("a", "a"));
    ASSERT_FALSE(DM("a", "b"));
    ASSERT_FALSE(DM("a", ""));
}

TEST(dfa_concat)
{
    ASSERT_TRUE(DM("ab", "ab"));
    ASSERT_FALSE(DM("ab", "a"));
    ASSERT_FALSE(DM("ab", "abc"));
}

TEST(dfa_union)
{
    ASSERT_TRUE(DM("a|b", "a"));
    ASSERT_TRUE(DM("a|b", "b"));
    ASSERT_FALSE(DM("a|b", "c"));
}

TEST(dfa_star)
{
    ASSERT_TRUE(DM("a*", ""));
    ASSERT_TRUE(DM("a*", "aaa"));
    ASSERT_FALSE(DM("a*", "b"));
}

TEST(dfa_plus)
{
    ASSERT_FALSE(DM("a+", ""));
    ASSERT_TRUE(DM("a+", "a"));
    ASSERT_TRUE(DM("a+", "aaaa"));
}

TEST(dfa_dot)
{
    ASSERT_TRUE(DM(".", "x"));
    ASSERT_FALSE(DM(".", ""));
    ASSERT_FALSE(DM(".", "xx"));
}

TEST(dfa_charclass)
{
    ASSERT_TRUE(DM("[a-z]", "m"));
    ASSERT_FALSE(DM("[a-z]", "A"));
}

TEST(dfa_complex)
{
    ASSERT_TRUE(DM("(a|b)*c", "ababc"));
    ASSERT_FALSE(DM("(a|b)*c", "ababd"));
}

TEST(dfa_valid_flag)
{
    auto d = compile("abc");
    ASSERT_TRUE(d.valid);
}

TEST(dfa_state_count_positive)
{
    auto d = compile("a|b|c");
    ASSERT_TRUE(d.states.size() > 0);
}

TEST(minimize_literal)
{
    ASSERT_TRUE(MM("a", "a"));
    ASSERT_FALSE(MM("a", "b"));
}

TEST(minimize_union)
{
    ASSERT_TRUE(MM("a|b", "a"));
    ASSERT_TRUE(MM("a|b", "b"));
    ASSERT_FALSE(MM("a|b", "c"));
}

TEST(minimize_reduces_states)
{
    auto d = compile("(a|a)*");
    auto min = dfa::minimize(d);
    ASSERT_TRUE(min.states.size() <= d.states.size());
}

TEST(minimize_star)
{
    ASSERT_TRUE(MM("a*", ""));
    ASSERT_TRUE(MM("a*", "aaa"));
    ASSERT_FALSE(MM("a*", "b"));
}

TEST(minimize_complex_agrees_with_dfa)
{
    std::string pat = "(a|b)+c";
    std::vector<std::string> inputs = {"ac", "bc", "abc", "c", "aabbc", "x"};
    auto d = compile(pat);
    auto min = dfa::minimize(d);
    for (auto& s : inputs) ASSERT_EQ(dfa::dfa_match(d, s), dfa::dfa_match(min, s));
}
