#include "bishop/match_result.hpp"
#include "bishop/regex.hpp"
#include "framework.hpp"

using bishop::Group;
using bishop::MatchResult;
using bishop::Regex;

TEST(match_result_default_not_matched)
{
    MatchResult r;
    ASSERT_FALSE(r.matched());
    ASSERT_FALSE((bool)r);
}

TEST(match_result_basic)
{
    std::string text = "hello";
    MatchResult r(text, 0, 5);
    ASSERT_TRUE(r.matched());
    ASSERT_EQ(r.str(), "hello");
    ASSERT_EQ(r.start(), 0u);
    ASSERT_EQ(r.end(), 5u);
    ASSERT_EQ(r.length(), 5u);
}

TEST(match_result_substring)
{
    std::string text = "foobar";
    MatchResult r(text, 3, 6);
    ASSERT_EQ(r.str(), "bar");
}

TEST(match_result_group_zero_is_full_match)
{
    std::string text = "abc";
    MatchResult r(text, 0, 3);
    ASSERT_EQ(r.group(0), "abc");
}

TEST(match_result_add_group)
{
    std::string text = "foobar";
    MatchResult r(text, 0, 6);
    Group g;
    g.start = 0;
    g.end = 3;
    g.matched = true;
    r.add_group(g);
    ASSERT_EQ(r.group(1), "foo");
    ASSERT_EQ(r.group_count(), 1u);
}

TEST(match_result_unmatched_group_empty)
{
    std::string text = "abc";
    MatchResult r(text, 0, 3);
    Group g;
    g.matched = false;
    r.add_group(g);
    ASSERT_EQ(r.group(1), "");
}

TEST(match_result_named_group)
{
    std::string text = "hello";
    MatchResult r(text, 0, 5);
    Group g;
    g.start = 0;
    g.end = 5;
    g.matched = true;
    r.add_group(g);
    r.add_named("word", 1);
    ASSERT_EQ(r.group("word"), "hello");
    ASSERT_TRUE(r.has_group("word"));
    ASSERT_FALSE(r.has_group("other"));
}

TEST(match_result_named_missing_returns_empty)
{
    std::string text = "abc";
    MatchResult r(text, 0, 3);
    ASSERT_EQ(r.group("missing"), "");
}

TEST(group_empty_default)
{
    Group g;
    ASSERT_TRUE(g.empty());
    ASSERT_EQ(g.length(), 0u);
}

TEST(group_matched_length)
{
    Group g;
    g.start = 2;
    g.end = 7;
    g.matched = true;
    ASSERT_FALSE(g.empty());
    ASSERT_EQ(g.length(), 5u);
}

TEST(match_result_multiple_groups)
{
    std::string text = "2024-01-15";
    MatchResult r(text, 0, 10);
    Group year;
    year.start = 0;
    year.end = 4;
    year.matched = true;
    Group month;
    month.start = 5;
    month.end = 7;
    month.matched = true;
    Group day;
    day.start = 8;
    day.end = 10;
    day.matched = true;
    r.add_group(year);
    r.add_group(month);
    r.add_group(day);
    r.add_named("year", 1);
    r.add_named("month", 2);
    r.add_named("day", 3);
    ASSERT_EQ(r.group("year"), "2024");
    ASSERT_EQ(r.group("month"), "01");
    ASSERT_EQ(r.group("day"), "15");
    ASSERT_EQ(r.group(1), "2024");
    ASSERT_EQ(r.group(2), "01");
    ASSERT_EQ(r.group(3), "15");
    ASSERT_EQ(r.group_count(), 3u);
}

TEST(regex_search_returns_match_result_str)
{
    Regex r("ab");
    auto [s, e] = r.search("xxabxx");
    ASSERT_EQ(s, 2u);
    ASSERT_EQ(e, 4u);
}
