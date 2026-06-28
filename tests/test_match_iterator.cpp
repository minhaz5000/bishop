#include <string>
#include <vector>

#include "bishop/match_iterator.hpp"
#include "bishop/regex.hpp"
#include "framework.hpp"

using bishop::MatchIterator;
using bishop::Regex;

TEST(match_iterator_empty_match)
{
    Regex r("z");
    std::string text = "abc";
    MatchIterator it(&r, text);
    MatchIterator end;
    ASSERT_TRUE(it == end);
}

TEST(match_iterator_multiple_matches)
{
    Regex r("a+");
    std::string text = "baaabaa";
    MatchIterator it(&r, text);
    MatchIterator end;

    ASSERT_TRUE(it != end);
    ASSERT_EQ(it->str(), "a");

    ++it;
    ASSERT_TRUE(it != end);
    ASSERT_EQ(it->str(), "aaa");

    ++it;
    ASSERT_TRUE(it != end);
    ASSERT_EQ(it->str(), "aa");

    ++it;
    ASSERT_TRUE(it == end);
}
