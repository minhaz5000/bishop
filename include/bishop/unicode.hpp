#ifndef BISHOP_UNICODE_HPP_
#define BISHOP_UNICODE_HPP_

#include <cstdint>
#include <string>
#include <vector>

namespace bishop {
namespace unicode {

struct Decoded {
    uint32_t codepoint{0};
    size_t   bytes{0};
};

Decoded decode_one(const std::string& s, size_t pos);

std::vector<uint32_t> decode_utf8(const std::string& s);

void encode_utf8(uint32_t cp, std::string& out);

bool is_valid_utf8(const std::string& s);

bool is_word_codepoint(uint32_t cp);
bool is_space_codepoint(uint32_t cp);
bool is_alpha_codepoint(uint32_t cp);

uint32_t fold_codepoint(uint32_t cp);

}  // namespace unicode
}  // namespace bishop
#endif
