#include <cstdint>
#include <string>

#include "bishop/unicode.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string s(reinterpret_cast<const char*>(data), size);

    bishop::unicode::is_valid_utf8(s);

    for (uint32_t cp : bishop::unicode::decode_utf8(s)) {
        bishop::unicode::is_word_codepoint(cp);
        bishop::unicode::is_space_codepoint(cp);
        bishop::unicode::fold_codepoint(cp);

        std::string roundtrip;
        bishop::unicode::encode_utf8(cp, roundtrip);
    }

    return 0;
}
