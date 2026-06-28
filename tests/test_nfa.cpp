#include "bishop/regex.hpp"
#include "framework.hpp"

using bishop::Regex;

static bool M(const std::string& pat, const std::string& text)
{
    Regex r(pat);
    return r.match(text);
}

TEST(literal_match)
{
    ASSERT_TRUE(M("a", "a"));
    ASSERT_FALSE(M("a", "b"));
    ASSERT_FALSE(M("a", ""));
}

TEST(concat_match)
{
    ASSERT_TRUE(M("ab", "ab"));
    ASSERT_FALSE(M("ab", "a"));
    ASSERT_FALSE(M("ab", "b"));
    ASSERT_FALSE(M("ab", "abc"));
}

TEST(union_match)
{
    ASSERT_TRUE(M("a|b", "a"));
    ASSERT_TRUE(M("a|b", "b"));
    ASSERT_FALSE(M("a|b", "c"));
    ASSERT_FALSE(M("a|b", "ab"));
}

TEST(kleene_star_match)
{
    ASSERT_TRUE(M("a*", ""));
    ASSERT_TRUE(M("a*", "a"));
    ASSERT_TRUE(M("a*", "aaa"));
    ASSERT_FALSE(M("a*", "b"));
}

TEST(plus_match)
{
    ASSERT_FALSE(M("a+", ""));
    ASSERT_TRUE(M("a+", "a"));
    ASSERT_TRUE(M("a+", "aaa"));
    ASSERT_FALSE(M("a+", "b"));
}

TEST(question_match)
{
    ASSERT_TRUE(M("a?", ""));
    ASSERT_TRUE(M("a?", "a"));
    ASSERT_FALSE(M("a?", "aa"));
}

TEST(dot_wildcard)
{
    ASSERT_TRUE(M(".", "a"));
    ASSERT_TRUE(M(".", "z"));
    ASSERT_TRUE(M(".", "1"));
    ASSERT_FALSE(M(".", ""));
    ASSERT_FALSE(M(".", "ab"));
}

TEST(charclass_match)
{
    ASSERT_TRUE(M("[abc]", "a"));
    ASSERT_TRUE(M("[abc]", "c"));
    ASSERT_FALSE(M("[abc]", "d"));
}

TEST(charclass_range_match)
{
    ASSERT_TRUE(M("[a-z]", "m"));
    ASSERT_FALSE(M("[a-z]", "A"));
    ASSERT_FALSE(M("[a-z]", "1"));
}

TEST(negated_charclass_match)
{
    ASSERT_FALSE(M("[^abc]", "a"));
    ASSERT_TRUE(M("[^abc]", "d"));
}

TEST(digit_shorthand)
{
    ASSERT_TRUE(M("\\d", "5"));
    ASSERT_FALSE(M("\\d", "a"));
}

TEST(word_shorthand)
{
    ASSERT_TRUE(M("\\w+", "hello123"));
    ASSERT_FALSE(M("\\w+", ""));
}

TEST(complex_pattern)
{
    ASSERT_TRUE(M("(a|b)*c", "c"));
    ASSERT_TRUE(M("(a|b)*c", "ac"));
    ASSERT_TRUE(M("(a|b)*c", "ababc"));
    ASSERT_FALSE(M("(a|b)*c", "abc"));
}

TEST(nested_groups)
{
    ASSERT_TRUE(M("(a(b|c))+", "ab"));
    ASSERT_TRUE(M("(a(b|c))+", "acabac"));
    ASSERT_FALSE(M("(a(b|c))+", "ad"));
}

TEST(search_finds_substring)
{
    Regex r("ab");
    auto [s, e] = r.search("xxabxx");
    ASSERT_EQ(s, 2u);
    ASSERT_EQ(e, 4u);
}

TEST(search_no_match_returns_npos)
{
    Regex r("xy");
    auto [s, e] = r.search("abcdef");
    ASSERT_EQ(s, std::string::npos);
}

TEST(anchor_start)
{
    Regex r("^abc");
    ASSERT_TRUE(r.search("abcxxx").first == 0);
    ASSERT_EQ(r.search("xxxabc").first, std::string::npos);
}

TEST(anchor_end)
{
    Regex r("abc$");
    ASSERT_NE(r.search("xxxabc").first, std::string::npos);
    ASSERT_EQ(r.search("abcxxx").first, std::string::npos);
}

TEST(replace_first)
{
    Regex r("ab");
    ASSERT_EQ(r.replace("xabxabx", "Y"), "xYxabx");
}

TEST(replace_all_matches)
{
    Regex r("ab");
    ASSERT_EQ(r.replace_all("ababab", "X"), "XXX");
}

TEST(find_all_non_overlapping)
{
    Regex r("a");
    auto v = r.find_all("banana");
    ASSERT_EQ(v.size(), 3u);
}
