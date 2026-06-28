#include "bishop/unicode.hpp"
#include "framework.hpp"

using namespace bishop::unicode;

TEST(ascii_roundtrip)
{
    auto cps = decode_utf8("Hello");
    ASSERT_EQ(cps.size(), 5u);
    ASSERT_EQ(cps[0], static_cast<uint32_t>('H'));

    std::string out;
    for (auto cp : cps) encode_utf8(cp, out);
    ASSERT_EQ(out, "Hello");
}

TEST(two_byte_sequence)
{
    std::string s = "\xC3\xA9";
    auto cps = decode_utf8(s);
    ASSERT_EQ(cps.size(), 1u);
    ASSERT_EQ(cps[0], 0xE9u);

    std::string out;
    encode_utf8(0xE9, out);
    ASSERT_EQ(out, s);
}

TEST(three_byte_sequence)
{
    std::string s = "\xE4\xB8\xAD";
    auto cps = decode_utf8(s);
    ASSERT_EQ(cps.size(), 1u);
    ASSERT_EQ(cps[0], 0x4E2Du);
}

TEST(four_byte_sequence)
{
    std::string s = "\xF0\x9F\x98\x80";
    auto cps = decode_utf8(s);
    ASSERT_EQ(cps.size(), 1u);
    ASSERT_EQ(cps[0], 0x1F600u);
}

TEST(truncated_sequence_is_replaced)
{
    std::string s = "\xE4\xB8";
    auto cps = decode_utf8(s);
    ASSERT_TRUE(!cps.empty());
    ASSERT_EQ(cps[0], 0xFFFDu);
}

TEST(invalid_continuation_is_replaced)
{
    std::string s = "\xC3\x28";
    auto cps = decode_utf8(s);
    ASSERT_EQ(cps[0], 0xFFFDu);
}

TEST(overlong_two_byte_is_replaced)
{
    std::string s = "\xC0\x80";
    auto cps = decode_utf8(s);
    ASSERT_EQ(cps[0], 0xFFFDu);
}

TEST(is_valid_utf8_accepts_wellformed)
{
    ASSERT_TRUE(is_valid_utf8("plain ascii"));
    ASSERT_TRUE(is_valid_utf8("\xC3\xA9\xE4\xB8\xAD"));
}

TEST(is_valid_utf8_rejects_truncated)
{
    ASSERT_FALSE(is_valid_utf8("\xE4\xB8"));
}

TEST(word_codepoint_classification)
{
    ASSERT_TRUE(is_word_codepoint('a'));
    ASSERT_TRUE(is_word_codepoint('Z'));
    ASSERT_TRUE(is_word_codepoint('7'));
    ASSERT_TRUE(is_word_codepoint('_'));
    ASSERT_FALSE(is_word_codepoint(' '));
    ASSERT_FALSE(is_word_codepoint('.'));
}

TEST(alpha_codepoint_covers_latin1)
{
    ASSERT_TRUE(is_alpha_codepoint(0xE9));
    ASSERT_FALSE(is_alpha_codepoint(0xD7));
}

TEST(space_codepoint_handles_ascii_and_nbsp)
{
    ASSERT_TRUE(is_space_codepoint(' '));
    ASSERT_TRUE(is_space_codepoint('\t'));
    ASSERT_TRUE(is_space_codepoint(0x00A0));
    ASSERT_FALSE(is_space_codepoint('x'));
}

TEST(fold_codepoint_ascii)
{
    ASSERT_EQ(fold_codepoint('A'), static_cast<uint32_t>('a'));
    ASSERT_EQ(fold_codepoint('z'), static_cast<uint32_t>('z'));
}

TEST(fold_codepoint_latin1_supplement)
{
    ASSERT_EQ(fold_codepoint(0xC9), 0xE9u);  // É -> é
}
