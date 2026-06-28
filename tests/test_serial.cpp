#include <sstream>

#include "bishop/dfa.hpp"
#include "bishop/nfa.hpp"
#include "bishop/serializer.hpp"
#include "bishop/tokenizer.hpp"
#include "framework.hpp"

using namespace tokenizer;

static dfa::DFA compile(const std::string& pat)
{
    auto t = tokenize(pat);
    strip_anchors(t);
    t = insert_concats(t);
    auto n = nfa::parse_postfix(to_postfix(t));
    return dfa::nfa_to_dfa(n);
}

TEST(binary_roundtrip_literal)
{
    auto d = compile("abc");
    std::ostringstream out;
    bishop::serial::serialize_dfa(d, out);
    std::istringstream in(out.str());
    auto d2 = bishop::serial::deserialize_dfa(in);
    ASSERT_EQ(d.states.size(), d2.states.size());
    ASSERT_EQ(d.start, d2.start);
    ASSERT_EQ(d.valid, d2.valid);
}

TEST(binary_roundtrip_match_preserved)
{
    auto d = compile("a+b");
    std::ostringstream out;
    bishop::serial::serialize_dfa(d, out);
    std::istringstream in(out.str());
    auto d2 = bishop::serial::deserialize_dfa(in);
    ASSERT_TRUE(dfa::dfa_match(d2, "ab"));
    ASSERT_TRUE(dfa::dfa_match(d2, "aaab"));
    ASSERT_FALSE(dfa::dfa_match(d2, "b"));
    ASSERT_FALSE(dfa::dfa_match(d2, ""));
}

TEST(binary_roundtrip_union)
{
    auto d = compile("cat|dog");
    std::ostringstream out;
    bishop::serial::serialize_dfa(d, out);
    std::istringstream in(out.str());
    auto d2 = bishop::serial::deserialize_dfa(in);
    ASSERT_TRUE(dfa::dfa_match(d2, "cat"));
    ASSERT_TRUE(dfa::dfa_match(d2, "dog"));
    ASSERT_FALSE(dfa::dfa_match(d2, "cow"));
}

TEST(binary_bad_magic_throws)
{
    std::istringstream in("BADMAGICDATA");
    ASSERT_THROWS(bishop::serial::deserialize_dfa(in));
}

TEST(text_roundtrip_literal)
{
    auto d = compile("hello");
    std::ostringstream out;
    bishop::serial::serialize_dfa_text(d, out);
    std::istringstream in(out.str());
    auto d2 = bishop::serial::deserialize_dfa_text(in);
    ASSERT_EQ(d.states.size(), d2.states.size());
    ASSERT_EQ(d.start, d2.start);
}

TEST(text_roundtrip_match_preserved)
{
    auto d = compile("[a-z]+");
    std::ostringstream out;
    bishop::serial::serialize_dfa_text(d, out);
    std::istringstream in(out.str());
    auto d2 = bishop::serial::deserialize_dfa_text(in);
    ASSERT_TRUE(dfa::dfa_match(d2, "hello"));
    ASSERT_FALSE(dfa::dfa_match(d2, "123"));
}

TEST(text_roundtrip_star)
{
    auto d = compile("a*");
    std::ostringstream out;
    bishop::serial::serialize_dfa_text(d, out);
    std::istringstream in(out.str());
    auto d2 = bishop::serial::deserialize_dfa_text(in);
    ASSERT_TRUE(dfa::dfa_match(d2, ""));
    ASSERT_TRUE(dfa::dfa_match(d2, "aaa"));
    ASSERT_FALSE(dfa::dfa_match(d2, "b"));
}

TEST(text_bad_header_throws)
{
    std::istringstream in("GARBAGE 1\n");
    ASSERT_THROWS(bishop::serial::deserialize_dfa_text(in));
}

TEST(binary_valid_flag_preserved)
{
    auto d = compile("x");
    std::ostringstream out;
    bishop::serial::serialize_dfa(d, out);
    std::istringstream in(out.str());
    auto d2 = bishop::serial::deserialize_dfa(in);
    ASSERT_EQ(d.valid, d2.valid);
}

TEST(text_output_contains_header)
{
    auto d = compile("a");
    std::ostringstream out;
    bishop::serial::serialize_dfa_text(d, out);
    ASSERT_TRUE(out.str().find("BISHOP_DFA") != std::string::npos);
}

TEST(text_output_contains_end)
{
    auto d = compile("a");
    std::ostringstream out;
    bishop::serial::serialize_dfa_text(d, out);
    ASSERT_TRUE(out.str().find("END") != std::string::npos);
}

TEST(minimize_then_roundtrip)
{
    auto d = compile("(a|b)+c");
    auto min = dfa::minimize(d);
    std::ostringstream out;
    bishop::serial::serialize_dfa(min, out);
    std::istringstream in(out.str());
    auto d2 = bishop::serial::deserialize_dfa(in);
    ASSERT_TRUE(dfa::dfa_match(d2, "ac"));
    ASSERT_TRUE(dfa::dfa_match(d2, "ababc"));
    ASSERT_FALSE(dfa::dfa_match(d2, "c"));
}
