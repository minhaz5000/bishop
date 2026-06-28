#include "bishop/splitter.hpp"
#include "framework.hpp"

using namespace bishop;
using namespace bishop::splitter;

TEST(split_single_delimiter)
{
    auto parts = split("hello world", " ");
    ASSERT_EQ(parts.size(), 2u);
    ASSERT_EQ(parts[0], "hello");
    ASSERT_EQ(parts[1], "world");
}

TEST(split_multiple_delimiters)
{
    auto parts = split("a,b,c,d", ",");
    ASSERT_EQ(parts.size(), 4u);
    ASSERT_EQ(parts[0], "a");
    ASSERT_EQ(parts[3], "d");
}

TEST(split_no_delimiter_returns_whole_string)
{
    auto parts = split("hello", ",");
    ASSERT_EQ(parts.size(), 1u);
    ASSERT_EQ(parts[0], "hello");
}

TEST(split_empty_string)
{
    auto parts = split("", ",");
    ASSERT_EQ(parts.size(), 1u);
    ASSERT_EQ(parts[0], "");
}

TEST(split_delimiter_at_start)
{
    auto parts = split(",hello", ",");
    ASSERT_EQ(parts.size(), 2u);
    ASSERT_EQ(parts[0], "");
    ASSERT_EQ(parts[1], "hello");
}

TEST(split_delimiter_at_end)
{
    auto parts = split("hello,", ",");
    ASSERT_EQ(parts.size(), 2u);
    ASSERT_EQ(parts[0], "hello");
    ASSERT_EQ(parts[1], "");
}

TEST(split_consecutive_delimiters)
{
    auto parts = split("a,,b", ",");
    ASSERT_EQ(parts.size(), 3u);
    ASSERT_EQ(parts[0], "a");
    ASSERT_EQ(parts[1], "");
    ASSERT_EQ(parts[2], "b");
}

TEST(split_regex_delimiter)
{
    auto parts = split("one  two   three", "[[:space:]]+");
    ASSERT_EQ(parts.size(), 3u);
    ASSERT_EQ(parts[0], "one");
    ASSERT_EQ(parts[1], "two");
    ASSERT_EQ(parts[2], "three");
}

TEST(split_digit_delimiter)
{
    auto parts = split("a1b2c", "[0-9]");
    ASSERT_EQ(parts.size(), 3u);
    ASSERT_EQ(parts[0], "a");
    ASSERT_EQ(parts[1], "b");
    ASSERT_EQ(parts[2], "c");
}

TEST(split_dot_delimiter)
{
    auto parts = split("1.2.3", "\\.");
    auto parts2 = split("1.2.3", "[.]");
    ASSERT_EQ(parts2.size(), 3u);
    ASSERT_EQ(parts2[0], "1");
    ASSERT_EQ(parts2[1], "2");
    ASSERT_EQ(parts2[2], "3");
}

TEST(split_multichar_delimiter)
{
    auto parts = split("aXXbXXc", "XX");
    ASSERT_EQ(parts.size(), 3u);
    ASSERT_EQ(parts[0], "a");
    ASSERT_EQ(parts[1], "b");
    ASSERT_EQ(parts[2], "c");
}

TEST(split_n_limit_two)
{
    auto parts = split_n("a,b,c,d", ",", 2);
    ASSERT_EQ(parts.size(), 2u);
    ASSERT_EQ(parts[0], "a");
    ASSERT_EQ(parts[1], "b,c,d");
}

TEST(split_n_limit_one)
{
    auto parts = split_n("a,b,c", ",", 1);
    ASSERT_EQ(parts.size(), 1u);
    ASSERT_EQ(parts[0], "a,b,c");
}

TEST(split_n_limit_larger_than_segments)
{
    auto parts = split_n("a,b", ",", 10);
    ASSERT_EQ(parts.size(), 2u);
    ASSERT_EQ(parts[0], "a");
    ASSERT_EQ(parts[1], "b");
}

TEST(split_n_limit_zero_same_as_split)
{
    auto unlimited = split("a,b,c", ",");
    auto zero_lim = split_n("a,b,c", ",", 0);
    ASSERT_EQ(unlimited.size(), zero_lim.size());
    for (size_t i = 0; i < unlimited.size(); ++i) ASSERT_EQ(unlimited[i], zero_lim[i]);
}

TEST(split_with_delimiters_count)
{
    auto segs = split_with_delimiters("a,b,c", ",");
    ASSERT_EQ(segs.size(), 5u);
}

TEST(split_with_delimiters_types)
{
    auto segs = split_with_delimiters("a,b", ",");
    ASSERT_FALSE(segs[0].is_delimiter);
    ASSERT_TRUE(segs[1].is_delimiter);
    ASSERT_FALSE(segs[2].is_delimiter);
}

TEST(split_with_delimiters_text)
{
    auto segs = split_with_delimiters("a,b", ",");
    ASSERT_EQ(segs[0].text, "a");
    ASSERT_EQ(segs[1].text, ",");
    ASSERT_EQ(segs[2].text, "b");
}

TEST(split_with_delimiters_offsets)
{
    auto segs = split_with_delimiters("a,b", ",");
    ASSERT_EQ(segs[0].offset, 0u);
    ASSERT_EQ(segs[1].offset, 1u);
    ASSERT_EQ(segs[2].offset, 2u);
}

TEST(split_with_delimiters_join_roundtrip)
{
    std::string orig = "hello, world! foo bar";
    auto segs = split_with_delimiters(orig, "[[:space:]]+");
    std::string reconstructed = Splitter::join(segs);
    ASSERT_EQ(reconstructed, orig);
}

TEST(split_with_delimiters_no_match)
{
    auto segs = split_with_delimiters("hello", ",");
    ASSERT_EQ(segs.size(), 1u);
    ASSERT_FALSE(segs[0].is_delimiter);
    ASSERT_EQ(segs[0].text, "hello");
}

TEST(split_with_delimiters_delimiter_at_start)
{
    auto segs = split_with_delimiters(",hello", ",");
    ASSERT_EQ(segs.size(), 3u);
    ASSERT_EQ(segs[0].text, "");
    ASSERT_TRUE(segs[1].is_delimiter);
    ASSERT_EQ(segs[2].text, "hello");
}

TEST(join_parts_with_separator)
{
    std::vector<std::string> parts{"a", "b", "c"};
    std::string result = Splitter::join(parts, ",");
    ASSERT_EQ(result, "a,b,c");
}

TEST(join_empty_separator)
{
    std::vector<std::string> parts{"foo", "bar"};
    std::string result = Splitter::join(parts, "");
    ASSERT_EQ(result, "foobar");
}

TEST(join_single_element)
{
    std::vector<std::string> parts{"only"};
    std::string result = Splitter::join(parts, ",");
    ASSERT_EQ(result, "only");
}

TEST(join_empty_vector)
{
    std::vector<std::string> parts{};
    std::string result = Splitter::join(parts, ",");
    ASSERT_EQ(result, "");
}

TEST(splitter_object_reuse)
{
    Splitter sp(",");
    auto p1 = sp.split("a,b");
    auto p2 = sp.split("x,y,z");
    ASSERT_EQ(p1.size(), 2u);
    ASSERT_EQ(p2.size(), 3u);
}

TEST(splitter_from_regex_object)
{
    Regex re("[0-9]+");
    Splitter sp(std::move(re));
    auto parts = sp.split("a1b22c333d");
    ASSERT_EQ(parts.size(), 4u);
    ASSERT_EQ(parts[0], "a");
    ASSERT_EQ(parts[1], "b");
    ASSERT_EQ(parts[2], "c");
    ASSERT_EQ(parts[3], "d");
}

TEST(splitter_regex_access)
{
    Splitter sp("[a-z]+");
    ASSERT_FALSE(sp.regex().pattern().empty());
}

TEST(split_only_delimiters)
{
    auto parts = split(",,,", ",");
    ASSERT_EQ(parts.size(), 4u);
    for (auto& p : parts) ASSERT_EQ(p, "");
}

TEST(split_single_char_string)
{
    auto parts = split("a", "a");
    ASSERT_EQ(parts.size(), 2u);
    ASSERT_EQ(parts[0], "");
    ASSERT_EQ(parts[1], "");
}

TEST(split_whitespace_pattern)
{
    auto parts = split("  hello   world  ", "\\s+");
    ASSERT_GE(parts.size(), 2u);
    bool found_hello = false, found_world = false;
    for (auto& p : parts) {
        if (p == "hello") found_hello = true;
        if (p == "world") found_world = true;
    }
    ASSERT_TRUE(found_hello);
    ASSERT_TRUE(found_world);
}
