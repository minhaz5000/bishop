#include "bishop/regex.hpp"
#include "framework.hpp"

using bishop::Flag;
using bishop::Regex;

static bool M(const std::string& p, const std::string& t)
{
    return Regex(p).match(t);
}

static bool S(const std::string& p, const std::string& t)
{
    return Regex(p).search(t).first != std::string::npos;
}

TEST(suite_integer_literal)
{
    ASSERT_TRUE(M("-?\\d+", "0"));
    ASSERT_TRUE(M("-?\\d+", "42"));
    ASSERT_TRUE(M("-?\\d+", "-7"));
    ASSERT_FALSE(M("-?\\d+", ""));
    ASSERT_FALSE(M("-?\\d+", "abc"));
}

TEST(suite_float_literal)
{
    Regex r("-?\\d+\\.\\d+");
    ASSERT_TRUE(r.match("3.14"));
    ASSERT_TRUE(r.match("-0.5"));
    ASSERT_FALSE(r.match("3"));
    ASSERT_FALSE(r.match(".14"));
}

TEST(suite_hex_literal)
{
    Regex r("0[xX][0-9a-fA-F]+");
    ASSERT_TRUE(r.match("0xFF"));
    ASSERT_TRUE(r.match("0x0"));
    ASSERT_TRUE(r.match("0XDEADBEEF"));
    ASSERT_FALSE(r.match("0xGG"));
    ASSERT_FALSE(r.match("FF"));
}

TEST(suite_identifier)
{
    Regex r("[a-zA-Z_][a-zA-Z0-9_]*");
    ASSERT_TRUE(r.match("foo"));
    ASSERT_TRUE(r.match("_bar"));
    ASSERT_TRUE(r.match("Baz123"));
    ASSERT_FALSE(r.match("123abc"));
    ASSERT_FALSE(r.match(""));
}

TEST(suite_ipv4_address)
{
    Regex r("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");
    ASSERT_TRUE(r.match("192.168.1.1"));
    ASSERT_TRUE(r.match("0.0.0.0"));
    ASSERT_TRUE(r.match("255.255.255.255"));
    ASSERT_FALSE(r.match("256.1.1.1"));
    ASSERT_FALSE(r.match("1.2.3"));
}

TEST(suite_email_simplified)
{
    Regex r("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}");
    ASSERT_TRUE(r.match("user@example.com"));
    ASSERT_TRUE(r.match("foo.bar+baz@sub.domain.org"));
    ASSERT_FALSE(r.match("notanemail"));
    ASSERT_FALSE(r.match("@nodomain.com"));
}

TEST(suite_url_simplified)
{
    Regex r("https?://[a-zA-Z0-9.-]+(/[a-zA-Z0-9._~:/?#\\[\\]@!$&'()*+,;=-]*)?");
    ASSERT_TRUE(r.match("https://example.com"));
    ASSERT_TRUE(r.match("http://foo.bar/path"));
    ASSERT_FALSE(r.match("ftp://nope.com"));
    ASSERT_FALSE(r.match("notaurl"));
}

TEST(suite_iso_date)
{
    Regex r("\\d{4}-\\d{2}-\\d{2}");
    ASSERT_TRUE(r.match("2024-01-15"));
    ASSERT_TRUE(r.match("1999-12-31"));
    ASSERT_FALSE(r.match("24-1-5"));
    ASSERT_FALSE(r.match("2024/01/15"));
}

TEST(suite_24h_time)
{
    Regex r("([01]\\d|2[0-3]):[0-5]\\d(:[0-5]\\d)?");
    ASSERT_TRUE(r.match("00:00"));
    ASSERT_TRUE(r.match("23:59"));
    ASSERT_TRUE(r.match("12:34:56"));
    ASSERT_FALSE(r.match("24:00"));
    ASSERT_FALSE(r.match("12:60"));
}

TEST(suite_c_string_literal)
{
    Regex r("\"([^\"\\\\]|\\\\.)*\"");
    ASSERT_TRUE(r.match("\"hello\""));
    ASSERT_TRUE(r.match("\"say \\\"hi\\\"\""));
    ASSERT_FALSE(r.match("hello"));
    ASSERT_FALSE(r.match("\"unterminated"));
}

TEST(suite_line_comment)
{
    Regex r("//[^\n]*");
    ASSERT_TRUE(r.match("// this is a comment"));
    ASSERT_TRUE(r.match("//"));
    ASSERT_FALSE(r.match("not a comment"));
}

TEST(suite_search_in_text)
{
    Regex r("\\d+");
    ASSERT_TRUE(S(r.pattern(), "abc 123 def"));
    ASSERT_TRUE(S(r.pattern(), "42"));
    ASSERT_FALSE(S(r.pattern(), "no digits here"));
}

TEST(suite_replace_first)
{
    Regex r("\\d+");
    ASSERT_EQ(r.replace("foo 42 bar 7", "NUM"), "foo NUM bar 7");
}

TEST(suite_replace_all)
{
    Regex r("\\d+");
    ASSERT_EQ(r.replace_all("1 and 2 and 3", "N"), "N and N and N");
}

TEST(suite_find_all_count)
{
    Regex r("[A-Z]");
    auto v = r.find_all("Hello World");
    ASSERT_EQ(v.size(), 2u);
}

TEST(suite_ignorecase_search)
{
    Regex r("hello", Flag::IGNORECASE);
    ASSERT_TRUE(S("hello", "Say HELLO there"));
}

TEST(suite_dotall_multiline)
{
    Regex r("begin.+end", Flag::DOTALL);
    ASSERT_TRUE(r.match("begin\nmiddle\nend"));
}

TEST(suite_word_boundary_in_text)
{
    Regex r("[a-z]+");
    auto v = r.find_all("the quick brown fox");
    ASSERT_EQ(v.size(), 4u);
}

TEST(suite_anchored_full_match)
{
    Regex r("^\\d+$");
    ASSERT_NE(r.search("12345").first, std::string::npos);
    ASSERT_EQ(r.search("12abc").first, std::string::npos);
}

TEST(suite_optional_group)
{
    Regex r("colou?r");
    ASSERT_TRUE(r.match("color"));
    ASSERT_TRUE(r.match("colour"));
    ASSERT_FALSE(r.match("colouur"));
}

TEST(suite_repeat_exact)
{
    Regex r("[A-Z]{3}\\d{4}");
    ASSERT_TRUE(r.match("ABC1234"));
    ASSERT_FALSE(r.match("AB123"));
    ASSERT_FALSE(r.match("ABCD12345"));
}

TEST(suite_alternation_words)
{
    Regex r("cat|dog|bird");
    ASSERT_TRUE(r.match("cat"));
    ASSERT_TRUE(r.match("dog"));
    ASSERT_TRUE(r.match("bird"));
    ASSERT_FALSE(r.match("fish"));
}

TEST(suite_posix_alpha_plus)
{
    Regex r("[[:alpha:]]+");
    ASSERT_TRUE(r.match("Hello"));
    ASSERT_FALSE(r.match("123"));
    ASSERT_FALSE(r.match(""));
}

TEST(suite_escape_dot_literal)
{
    Regex r("\\d\\.\\d");
    ASSERT_TRUE(r.match("3.5"));
    ASSERT_FALSE(r.match("3x5"));
}
