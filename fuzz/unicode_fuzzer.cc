#include <cstdint>
#include <string>
#include <vector>

#include "bishop/unicode.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0) return 0;

    const std::string s(reinterpret_cast<const char*>(data), size);

    const bool valid = bishop::unicode::is_valid_utf8(s);
    const auto codepoints = bishop::unicode::decode_utf8(s);

    if (valid) {
        std::string roundtrip;
        for (uint32_t cp : codepoints) bishop::unicode::encode_utf8(cp, roundtrip);
        if (!bishop::unicode::is_valid_utf8(roundtrip)) __builtin_trap();
    }

    for (uint32_t cp : codepoints) {
        const bool is_word = bishop::unicode::is_word_codepoint(cp);
        const bool is_space = bishop::unicode::is_space_codepoint(cp);

        if (is_word && is_space) __builtin_trap();

        const uint32_t folded = bishop::unicode::fold_codepoint(cp);
        const uint32_t folded2 = bishop::unicode::fold_codepoint(folded);
        if (folded != folded2) __builtin_trap();

        std::string enc;
        bishop::unicode::encode_utf8(cp, enc);
        if (!enc.empty() && !bishop::unicode::is_valid_utf8(enc)) __builtin_trap();

        if (!enc.empty()) {
            auto rcp = bishop::unicode::decode_utf8(enc);
            (void)rcp;
        }
    }

    return 0;
}
