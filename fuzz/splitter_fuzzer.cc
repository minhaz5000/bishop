#include <cstdint>
#include <string>

#include "bishop/error.hpp"
#include "bishop/flags.hpp"
#include "bishop/splitter.hpp"

static constexpr size_t kMaxPattern = 256;
static constexpr size_t kMaxText = 2048;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 3) return 0;

    const uint8_t flag_byte = data[0];
    const uint8_t limit_byte = data[1];
    bishop::Flag flags = bishop::Flag::NONE;
    if (flag_byte & 0x01) flags |= bishop::Flag::IGNORECASE;
    if (flag_byte & 0x02) flags |= bishop::Flag::MULTILINE;
    const size_t limit = static_cast<size_t>(limit_byte % 16) + 1;

    const char* body = reinterpret_cast<const char*>(data + 2);
    const size_t bsize = size - 2;

    size_t sep = bsize;
    for (size_t i = 0; i < bsize; ++i) {
        if (body[i] == '\n') {
            sep = i;
            break;
        }
    }

    const std::string pattern(body, std::min(sep, kMaxPattern));
    const size_t text_off = (sep < bsize) ? sep + 1 : bsize;
    const std::string text(body + text_off, std::min(bsize - text_off, kMaxText));

    try {
        bishop::splitter::Splitter sp(pattern, flags);

        auto parts = sp.split(text);
        auto parts_n = sp.split_n(text, limit);
        auto segs = sp.split_with_delimiters(text);

        (void)bishop::splitter::Splitter::join(segs);
        (void)bishop::splitter::Splitter::join(parts, "|");
        (void)bishop::splitter::Splitter::join(parts_n, ",");

        auto rejoined = bishop::splitter::Splitter::join(segs);
        (void)rejoined;

        (void)sp.split("");
        (void)sp.split_n("", limit);
        (void)sp.split_with_delimiters("");

        (void)bishop::splitter::split(text, pattern, flags);
        (void)bishop::splitter::split_n(text, pattern, limit, flags);
        (void)bishop::splitter::split_with_delimiters(text, pattern, flags);
    }
    catch (const bishop::RegexError&) {
    }
    catch (const std::exception&) {
    }

    return 0;
}
