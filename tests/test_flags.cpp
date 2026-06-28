#include "bishop/flags.hpp"
#include "bishop/regex.hpp"
#include "framework.hpp"

using bishop::Flag;
using bishop::Regex;

TEST(flag_none_is_zero)
{
    ASSERT_EQ(static_cast<uint32_t>(Flag::NONE), 0u);
}

TEST(flag_or_combines)
{
    Flag f = Flag::IGNORECASE | Flag::MULTILINE;
    ASSERT_TRUE(bishop::has_flag(f, Flag::IGNORECASE));
    ASSERT_TRUE(bishop::has_flag(f, Flag::MULTILINE));
    ASSERT_FALSE(bishop::has_flag(f, Flag::DOTALL));
}

TEST(flag_and_masks)
{
    Flag f = Flag::IGNORECASE | Flag::DOTALL;
    ASSERT_TRUE(bishop::has_flag(f & Flag::DOTALL, Flag::DOTALL));
    ASSERT_FALSE(bishop::has_flag(f & Flag::MULTILINE, Flag::MULTILINE));
}

TEST(flags_to_string_all)
{
    Flag f = Flag::IGNORECASE | Flag::MULTILINE | Flag::DOTALL | Flag::EXTENDED | Flag::GLOBAL;
    auto s = bishop::flags_to_string(f);
    ASSERT_TRUE(s.find('i') != std::string::npos);
    ASSERT_TRUE(s.find('m') != std::string::npos);
    ASSERT_TRUE(s.find('s') != std::string::npos);
    ASSERT_TRUE(s.find('x') != std::string::npos);
    ASSERT_TRUE(s.find('g') != std::string::npos);
}

TEST(flags_from_string_round_trip)
{
    std::string s = "ims";
    Flag f = bishop::flags_from_string(s);
    ASSERT_TRUE(bishop::has_flag(f, Flag::IGNORECASE));
    ASSERT_TRUE(bishop::has_flag(f, Flag::MULTILINE));
    ASSERT_TRUE(bishop::has_flag(f, Flag::DOTALL));
    ASSERT_FALSE(bishop::has_flag(f, Flag::EXTENDED));
}

TEST(flags_from_string_empty)
{
    ASSERT_EQ(bishop::flags_from_string(""), Flag::NONE);
}

TEST(ignorecase_matches_upper_and_lower)
{
    Regex r("hello", Flag::IGNORECASE);
    ASSERT_TRUE(r.match("hello"));
    ASSERT_TRUE(r.match("HELLO"));
    ASSERT_TRUE(r.match("Hello"));
    ASSERT_TRUE(r.match("hElLo"));
}

TEST(ignorecase_charclass)
{
    Regex r("[a-z]+", Flag::IGNORECASE);
    ASSERT_TRUE(r.match("abc"));
    ASSERT_TRUE(r.match("ABC"));
    ASSERT_TRUE(r.match("AbC"));
}

TEST(no_ignorecase_is_case_sensitive)
{
    Regex r("hello");
    ASSERT_TRUE(r.match("hello"));
    ASSERT_FALSE(r.match("HELLO"));
}

TEST(dotall_matches_newline)
{
    Regex r("a.b", Flag::DOTALL);
    ASSERT_TRUE(r.match("a\nb"));
    ASSERT_TRUE(r.match("a\rb"));
}

TEST(no_dotall_dot_skips_newline)
{
    Regex r("a.b");
    ASSERT_FALSE(r.match("a\nb"));
    ASSERT_TRUE(r.match("axb"));
}

TEST(extended_ignores_whitespace)
{
    Regex r("a b c", Flag::EXTENDED);
    ASSERT_TRUE(r.match("abc"));
}

TEST(extended_ignores_comments)
{
    Regex r("a # match a\nb # match b\n", Flag::EXTENDED);
    ASSERT_TRUE(r.match("ab"));
}

TEST(flags_stored_on_regex)
{
    Regex r("x", Flag::IGNORECASE | Flag::DOTALL);
    ASSERT_TRUE(bishop::has_flag(r.flags(), Flag::IGNORECASE));
    ASSERT_TRUE(bishop::has_flag(r.flags(), Flag::DOTALL));
    ASSERT_FALSE(bishop::has_flag(r.flags(), Flag::MULTILINE));
}

TEST(compile_updates_flags)
{
    Regex r("a");
    r.compile("b", Flag::IGNORECASE);
    ASSERT_TRUE(bishop::has_flag(r.flags(), Flag::IGNORECASE));
}
