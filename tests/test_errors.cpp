#include "bishop/error.hpp"
#include "bishop/regex.hpp"
#include "framework.hpp"

using bishop::CompileError;
using bishop::ParseError;
using bishop::Regex;
using bishop::RegexError;

TEST(empty_pattern_does_not_throw)
{
    ASSERT_NO_THROW(Regex r(""));
}

TEST(valid_patterns_do_not_throw)
{
    ASSERT_NO_THROW(Regex r("a*b+c?"));
    ASSERT_NO_THROW(Regex r("[a-z]+"));
    ASSERT_NO_THROW(Regex r("(a|b){2,4}"));
    ASSERT_NO_THROW(Regex r("\\d+\\.\\d+"));
}

TEST(pattern_stored_after_compile)
{
    Regex r("hello");
    ASSERT_EQ(r.pattern(), "hello");
}

TEST(recompile_updates_pattern)
{
    Regex r("first");
    r.compile("second");
    ASSERT_EQ(r.pattern(), "second");
}

TEST(regex_error_is_base_of_compile_error)
{
    try {
        throw bishop::CompileError("test");
    }
    catch (const bishop::RegexError&) {
    }
}

TEST(parse_error_carries_position)
{
    bishop::ParseError e("bad token", 42u);
    ASSERT_EQ(e.position(), 42u);
    std::string msg = e.what();
    ASSERT_FALSE(msg.empty());
}

TEST(match_after_compile_uses_new_pattern)
{
    Regex r("aaa");
    ASSERT_FALSE(r.match("aa"));
    r.compile("aa");
    ASSERT_TRUE(r.match("aa"));
}
