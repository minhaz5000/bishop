#include "framework.hpp"
#include "bishop/posix.hpp"
#include "bishop/regex.hpp"

using bishop::Regex;
using namespace bishop::posix;

TEST(posix_alpha_letters_only) {
    auto cs = cls_alpha();
    ASSERT_TRUE(cs['a']); ASSERT_TRUE(cs['Z']);
    ASSERT_FALSE(cs['0']); ASSERT_FALSE(cs[' ']);
}

TEST(posix_digit_digits_only) {
    auto cs = cls_digit();
    ASSERT_TRUE(cs['0']); ASSERT_TRUE(cs['9']);
    ASSERT_FALSE(cs['a']); ASSERT_FALSE(cs[' ']);
}

TEST(posix_alnum) {
    auto cs = cls_alnum();
    ASSERT_TRUE(cs['a']); ASSERT_TRUE(cs['5']);
    ASSERT_FALSE(cs[' ']); ASSERT_FALSE(cs['!']);
}

TEST(posix_space) {
    auto cs = cls_space();
    ASSERT_TRUE(cs[' ']); ASSERT_TRUE(cs['\t']); ASSERT_TRUE(cs['\n']);
    ASSERT_FALSE(cs['a']);
}

TEST(posix_upper_lower) {
    auto up = cls_upper();
    auto lo = cls_lower();
    ASSERT_TRUE(up['A']); ASSERT_FALSE(up['a']);
    ASSERT_TRUE(lo['a']); ASSERT_FALSE(lo['A']);
}

TEST(posix_punct) {
    auto cs = cls_punct();
    ASSERT_TRUE(cs['!']); ASSERT_TRUE(cs['.']); ASSERT_TRUE(cs[',']);
    ASSERT_FALSE(cs['a']); ASSERT_FALSE(cs['1']);
}

TEST(posix_xdigit) {
    auto cs = cls_xdigit();
    ASSERT_TRUE(cs['0']); ASSERT_TRUE(cs['9']);
    ASSERT_TRUE(cs['a']); ASSERT_TRUE(cs['f']);
    ASSERT_TRUE(cs['A']); ASSERT_TRUE(cs['F']);
    ASSERT_FALSE(cs['g']); ASSERT_FALSE(cs['G']);
}

TEST(posix_blank) {
    auto cs = cls_blank();
    ASSERT_TRUE(cs[' ']); ASSERT_TRUE(cs['\t']);
    ASSERT_FALSE(cs['\n']); ASSERT_FALSE(cs['a']);
}

TEST(posix_from_name_unknown_empty) {
    auto cs = from_name("bogus");
    for (int i = 0; i < 256; ++i) ASSERT_FALSE(cs[i]);
}

TEST(posix_class_in_charclass_alpha) {
    Regex r("[[:alpha:]]+");
    ASSERT_TRUE(r.match("abc"));
    ASSERT_TRUE(r.match("XYZ"));
    ASSERT_FALSE(r.match("123"));
    ASSERT_FALSE(r.match(""));
}

TEST(posix_class_in_charclass_digit) {
    Regex r("[[:digit:]]+");
    ASSERT_TRUE(r.match("0123"));
    ASSERT_FALSE(r.match("abc"));
}

TEST(posix_class_mixed_with_literal) {
    Regex r("[[:digit:]a-f]+");
    ASSERT_TRUE(r.match("09af"));
    ASSERT_TRUE(r.match("abc"));
    ASSERT_FALSE(r.match("xyz"));
}

TEST(posix_class_xdigit) {
    Regex r("[[:xdigit:]]+");
    ASSERT_TRUE(r.match("deadbeef"));
    ASSERT_TRUE(r.match("CAFE1234"));
    ASSERT_FALSE(r.match("xyz"));
}

TEST(posix_class_punct) {
    Regex r("[[:punct:]]");
    ASSERT_TRUE(r.match("!"));
    ASSERT_TRUE(r.match("."));
    ASSERT_FALSE(r.match("a"));
}
