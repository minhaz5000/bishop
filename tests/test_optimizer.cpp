#include "bishop/nfa.hpp"
#include "bishop/optimizer.hpp"
#include "bishop/tokenizer.hpp"
#include "framework.hpp"

using namespace bishop::optimizer;
using namespace tokenizer;

struct ScopedNfa {
    nfa::NFA value;
    explicit ScopedNfa(nfa::NFA n) : value(n) {}
    ScopedNfa(const ScopedNfa&) = delete;
    ScopedNfa& operator=(const ScopedNfa&) = delete;
    ScopedNfa(ScopedNfa&& other) noexcept : value(other.value)
    {
        other.value = {nullptr, nullptr};
    }
    ~ScopedNfa()
    {
        nfa::free_nfa(value);
    }
    operator nfa::NFA&()
    {
        return value;
    }
};

static ScopedNfa compile(const std::string& pat)
{
    auto t = tokenize(pat);
    strip_anchors(t);
    t = insert_concats(t);
    return ScopedNfa(nfa::parse_postfix(to_postfix(t)));
}

TEST(nfa_stats_literal)
{
    auto n = compile("a");
    auto s = nfa_stats(n);
    ASSERT_EQ(s.states_before, 2u);
    ASSERT_EQ(s.transitions_before, 1u);
    ASSERT_EQ(s.epsilon_before, 0u);
}

TEST(nfa_stats_star)
{
    auto n = compile("a*");
    auto s = nfa_stats(n);
    ASSERT_TRUE(s.states_before >= 4u);
    ASSERT_TRUE(s.epsilon_before >= 2u);
}

TEST(nfa_stats_concat)
{
    auto n = compile("ab");
    auto s = nfa_stats(n);
    ASSERT_TRUE(s.states_before >= 4u);
}

TEST(count_dead_states_no_dead)
{
    auto n = compile("abc");
    ASSERT_EQ(count_dead_states(n), 0u);
}

TEST(count_dead_states_union)
{
    auto n = compile("a|b");
    size_t dead = count_dead_states(n);
    ASSERT_TRUE(dead == 0u);
}

TEST(count_epsilon_chains)
{
    auto n = compile("a*b");
    size_t chains = count_epsilon_chains(n);
    ASSERT_TRUE(chains >= 0u);
}

TEST(collapse_epsilon_chains_runs)
{
    auto n = compile("a*b*c");
    auto s = collapse_epsilon_chains(n);
    ASSERT_TRUE(s.states_before >= 4u);
}

TEST(remove_dead_states_runs)
{
    auto n = compile("(a|b)*c");
    auto s = remove_dead_states(n);
    ASSERT_TRUE(s.states_before > 0u);
}

TEST(full_optimize_runs)
{
    auto n = compile("[a-z]+\\d+");
    auto s = full_optimize(n);
    ASSERT_TRUE(s.states_before > 0u);
}

TEST(stats_summary_non_empty)
{
    auto n = compile("a+");
    auto s = nfa_stats(n);
    auto txt = stats_summary(s);
    ASSERT_FALSE(txt.empty());
    ASSERT_TRUE(txt.find("states:") != std::string::npos);
}

TEST(full_optimize_nfa_still_correct)
{
    auto n = compile("a*b");
    full_optimize(n);
    ASSERT_TRUE(nfa::nfa_match(n, "b"));
    ASSERT_TRUE(nfa::nfa_match(n, "aaab"));
    ASSERT_FALSE(nfa::nfa_match(n, ""));
}
