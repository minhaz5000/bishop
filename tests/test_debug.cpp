#include "framework.hpp"
#include "bishop/debug.hpp"
#include "bishop/dfa.hpp"
#include "bishop/nfa.hpp"
#include "bishop/tokenizer.hpp"

using namespace tokenizer;
using bishop::debug::nfa_state_count;
using bishop::debug::nfa_transition_count;
using bishop::debug::nfa_to_dot;
using bishop::debug::dfa_to_dot;
using bishop::debug::print_nfa_text;
using bishop::debug::print_dfa_text;

static nfa::NFA compile_nfa(const std::string& pat) {
    auto t = tokenize(pat);
    strip_anchors(t);
    t = insert_concats(t);
    return nfa::parse_postfix(to_postfix(t));
}

static dfa::DFA compile_dfa(const std::string& pat) {
    auto n = compile_nfa(pat);
    return dfa::nfa_to_dfa(n);
}

TEST(nfa_state_count_literal) {
    auto n = compile_nfa("a");
    ASSERT_EQ(nfa_state_count(n), 2u);
}

TEST(nfa_state_count_concat) {
    auto n = compile_nfa("ab");
    ASSERT_TRUE(nfa_state_count(n) >= 4u);
}

TEST(nfa_state_count_star) {
    auto n = compile_nfa("a*");
    ASSERT_TRUE(nfa_state_count(n) >= 4u);
}

TEST(nfa_transition_count_literal) {
    auto n = compile_nfa("a");
    ASSERT_EQ(nfa_transition_count(n), 1u);
}

TEST(nfa_transition_count_union) {
    auto n = compile_nfa("a|b");
    ASSERT_TRUE(nfa_transition_count(n) >= 2u);
}

TEST(nfa_to_dot_contains_digraph) {
    auto n   = compile_nfa("ab");
    auto dot = nfa_to_dot(n);
    ASSERT_TRUE(dot.find("digraph") != std::string::npos);
}

TEST(nfa_to_dot_contains_start) {
    auto n   = compile_nfa("a");
    auto dot = nfa_to_dot(n, "TestNFA");
    ASSERT_TRUE(dot.find("TestNFA") != std::string::npos);
    ASSERT_TRUE(dot.find("start") != std::string::npos);
}

TEST(nfa_to_dot_contains_epsilon) {
    auto n   = compile_nfa("a*");
    auto dot = nfa_to_dot(n);
    ASSERT_TRUE(dot.find("ε") != std::string::npos);
}

TEST(dfa_to_dot_contains_digraph) {
    auto d   = compile_dfa("a|b");
    auto dot = dfa_to_dot(d);
    ASSERT_TRUE(dot.find("digraph") != std::string::npos);
}

TEST(dfa_to_dot_custom_name) {
    auto d   = compile_dfa("abc");
    auto dot = dfa_to_dot(d, "MyDFA");
    ASSERT_TRUE(dot.find("MyDFA") != std::string::npos);
}

TEST(print_nfa_text_writes_something) {
    auto n = compile_nfa("ab");
    std::ostringstream oss;
    print_nfa_text(n, oss);
    ASSERT_FALSE(oss.str().empty());
    ASSERT_TRUE(oss.str().find("NFA") != std::string::npos);
}

TEST(print_dfa_text_writes_something) {
    auto d = compile_dfa("a*b");
    std::ostringstream oss;
    print_dfa_text(d, oss);
    ASSERT_FALSE(oss.str().empty());
    ASSERT_TRUE(oss.str().find("DFA") != std::string::npos);
}

TEST(nfa_state_count_charclass) {
    auto n = compile_nfa("[a-z]+");
    ASSERT_TRUE(nfa_state_count(n) >= 4u);
}

TEST(nfa_state_count_dot) {
    auto n = compile_nfa(".");
    ASSERT_EQ(nfa_state_count(n), 2u);
}
