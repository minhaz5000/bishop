#include "bishop/unicode.hpp"

namespace bishop {
namespace unicode {

Decoded decode_one(const std::string& s, size_t pos) {
    if (pos >= s.size()) return {0, 0};

    unsigned char c0 = static_cast<unsigned char>(s[pos]);

    if (c0 < 0x80) return {c0, 1};

    size_t   extra;
    uint32_t cp;

    if ((c0 & 0xE0) == 0xC0) {
        extra = 1;
        cp    = c0 & 0x1F;
    } else if ((c0 & 0xF0) == 0xE0) {
        extra = 2;
        cp    = c0 & 0x0F;
    } else if ((c0 & 0xF8) == 0xF0) {
        extra = 3;
        cp    = c0 & 0x07;
    } else {
        return {0xFFFD, 1};
    }

    if (pos + extra >= s.size()) return {0xFFFD, 1};

    for (size_t k = 1; k <= extra; ++k) {
        unsigned char cc = static_cast<unsigned char>(s[pos + k]);
        if ((cc & 0xC0) != 0x80) return {0xFFFD, 1};
        cp = (cp << 6) | (cc & 0x3F);
    }

    if (extra == 1 && cp < 0x80)   return {0xFFFD, 1};
    if (extra == 2 && cp < 0x800)  return {0xFFFD, 1};
    if (extra == 3 && cp < 0x10000) return {0xFFFD, 1};

    return {cp, extra + 1};
}

std::vector<uint32_t> decode_utf8(const std::string& s) {
    std::vector<uint32_t> out;
    out.reserve(s.size());
    size_t pos = 0;
    while (pos < s.size()) {
        Decoded d = decode_one(s, pos);
        out.push_back(d.codepoint);
        pos += (d.bytes ? d.bytes : 1);
    }
    return out;
}

void encode_utf8(uint32_t cp, std::string& out) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

bool is_valid_utf8(const std::string& s) {
    size_t pos = 0;
    while (pos < s.size()) {
        Decoded d = decode_one(s, pos);
        if (d.bytes == 0 || d.codepoint == 0xFFFD) return false;
        pos += d.bytes;
    }
    return true;
}

namespace {

constexpr size_t kBmpSize = 0x10000;

const std::vector<bool>& alpha_bmp_table() {
    static const std::vector<bool> table = [] {
        std::vector<bool> t(kBmpSize, false);
        for (uint32_t c = 'A'; c <= 'Z'; ++c) t[c] = true;
        for (uint32_t c = 'a'; c <= 'z'; ++c) t[c] = true;
        for (uint32_t c = 0xC0; c <= 0xFF; ++c)
            if (c != 0xD7 && c != 0xF7) t[c] = true;          // Latin-1 letters
        for (uint32_t c = 0x100; c <= 0x24F; ++c) t[c] = true; // Latin Ext A/B
        for (uint32_t c = 0x370; c <= 0x3FF; ++c) t[c] = true; // Greek & Coptic
        for (uint32_t c = 0x400; c <= 0x4FF; ++c) t[c] = true; // Cyrillic
        for (uint32_t c = 0x590; c <= 0x5FF; ++c) t[c] = true; // Hebrew
        for (uint32_t c = 0x600; c <= 0x6FF; ++c) t[c] = true; // Arabic
        for (uint32_t c = 0x900; c <= 0x97F; ++c) t[c] = true; // Devanagari
        for (uint32_t c = 0x980; c <= 0x9FF; ++c) t[c] = true; // Bengali
        for (uint32_t c = 0x4E00; c <= 0x9FFF; ++c) t[c] = true; // CJK Unified
        for (uint32_t c = 0x3040; c <= 0x30FF; ++c) t[c] = true; // Hiragana/Katakana
        return t;
    }();
    return table;
}

}  // namespace

bool is_alpha_codepoint(uint32_t cp) {
    if (cp < 0x80) return (cp >= 'A' && cp <= 'Z') || (cp >= 'a' && cp <= 'z');
    return alpha_bmp_table()[cp];
}

bool is_space_codepoint(uint32_t cp) {
    switch (cp) {
        case ' ': case '\t': case '\n': case '\r': case '\f': case '\v':
        case 0x00A0:  // NBSP
        case 0x2028:  // line separator
        case 0x2029:  // paragraph separator
            return true;
        default:
            return cp >= 0x2000 && cp <= 0x200A;  // general punctuation spaces
    }
}

bool is_word_codepoint(uint32_t cp) {
    if (cp == '_') return true;
    if (cp >= '0' && cp <= '9') return true;
    return is_alpha_codepoint(cp);
}

uint32_t fold_codepoint(uint32_t cp) {
    if (cp >= 'A' && cp <= 'Z') return cp + ('a' - 'A');

    if (cp >= 0xC0 && cp <= 0xDE && cp != 0xD7) return cp + 0x20;

    if (cp >= 0x100 && cp <= 0x137 && (cp % 2) == 0) return cp + 1;
    if (cp >= 0x139 && cp <= 0x148 && (cp % 2) == 1) return cp + 1;
    if (cp >= 0x14A && cp <= 0x177 && (cp % 2) == 0) return cp + 1;

    return cp;
}

}  // namespace unicode
}  // namespace bishop
