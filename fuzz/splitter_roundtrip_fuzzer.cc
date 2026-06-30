#include <cstdint>
#include <cstdlib>
#include <string>

#include "bishop/error.hpp"
#include "bishop/flags.hpp"
#include "bishop/splitter.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 2) return 0;

    const uint8_t flag_byte = data[0];
    bishop::Flag flags = bishop::Flag::NONE;
    if (flag_byte & 0x01) flags |= bishop::Flag::IGNORECASE;
    if (flag_byte & 0x02) flags |= bishop::Flag::MULTILINE;

    const char* body = reinterpret_cast<const char*>(data + 1);
    const size_t body_size = size - 1;

    size_t sep = body_size;
    for (size_t i = 0; i < body_size; ++i) {
        if (body[i] == '\0') {
            sep = i;
            break;
        }
    }

    static constexpr size_t kMaxPattern = 64;
    static constexpr size_t kMaxText = 512;

    std::string pattern, text;
    if (sep < body_size) {
        pattern.assign(body, std::min(sep, kMaxPattern));
        text.assign(body + sep + 1, std::min(body_size - sep - 1, kMaxText));
    } else {
        static const char* fallback_patterns[] = {",", "[[:space:]]+", "[0-9]+", "\\.", "XX"};
        pattern = fallback_patterns[data[0] % 5];
        text.assign(body, std::min(body_size, kMaxText));
    }

    if (pattern.empty()) return 0;

    try {
        bishop::splitter::Splitter sp(pattern, flags);
        auto segs = sp.split_with_delimiters(text);

        std::string reconstructed = bishop::splitter::Splitter::join(segs);
        if (reconstructed != text) {
            std::abort();
        }

        auto simple = sp.split(text);
        size_t simple_idx = 0;
        for (const auto& seg : segs) {
            if (seg.is_delimiter) continue;
            if (simple_idx >= simple.size()) std::abort();
            if (simple[simple_idx] != seg.text) std::abort();
            ++simple_idx;
        }
        if (simple_idx != simple.size()) std::abort();

        const size_t limit = static_cast<size_t>(flag_byte % 16) + 1;
        auto parts_n = sp.split_n(text, limit);
        (void)bishop::splitter::Splitter::join(parts_n, ",");
        (void)bishop::splitter::Splitter::join(simple, "|");
        (void)sp.split("");
        (void)sp.split_n("", limit);
        (void)sp.split_with_delimiters("");
        (void)bishop::splitter::split(text, pattern, flags);
        (void)bishop::splitter::split_n(text, pattern, limit, flags);
        (void)bishop::splitter::split_with_delimiters(text, pattern, flags);
    }
    catch (const std::exception&) {
    }

    return 0;
}
