#include <cstdint>
#include <cstdlib>
#include <string>

#include "bishop/error.hpp"
#include "bishop/flags.hpp"
#include "bishop/regex.hpp"
#include "bishop/rewriter.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 2) return 0;

    const uint8_t flag_byte = data[0];
    bishop::Flag flags = bishop::Flag::NONE;
    if (flag_byte & 0x01) flags |= bishop::Flag::IGNORECASE;
    if (flag_byte & 0x02) flags |= bishop::Flag::MULTILINE;
    if (flag_byte & 0x04) flags |= bishop::Flag::GLOBAL;

    const char* body = reinterpret_cast<const char*>(data + 1);
    const size_t body_size = size - 1;

    size_t sep = body_size;
    for (size_t i = 0; i < body_size; ++i) {
        if (body[i] == '\0') {
            sep = i;
            break;
        }
    }
    if (sep >= body_size) return 0;

    static constexpr size_t kMaxPattern = 64;
    static constexpr size_t kMaxText = 512;

    const std::string pattern(body, std::min(sep, kMaxPattern));
    const std::string text(body + sep + 1, std::min(body_size - sep - 1, kMaxText));

    if (pattern.empty()) return 0;

    try {
        bishop::Regex re(pattern, flags);

        std::string all_result = bishop::rewriter::replace_all(re, text, "\\0");
        if (all_result != text) {
            std::abort();
        }

        std::string first_result = bishop::rewriter::replace_first(re, text, "\\0");
        if (first_result != text) {
            std::abort();
        }

        auto md = re.search_match(text);
        if (md.matched) {
            auto parts = bishop::rewriter::parse_replacement("\\0");
            std::string applied = bishop::rewriter::apply_replacement(parts, md, text);
            std::string expected_match = text.substr(md.start, md.end - md.start);
            if (applied != expected_match) {
                std::abort();
            }
        }

        auto extra_parts = bishop::rewriter::parse_replacement(pattern);
        (void)bishop::rewriter::dump_replacement(extra_parts);
        (void)bishop::rewriter::replace_fn(re, text, [](const nfa::MatchData& m, const std::string&) -> std::string {
            return m.matched ? std::string(m.end - m.start, 'X') : "";
        });
        (void)bishop::rewriter::replace_first(re, "", "\\0");
        (void)bishop::rewriter::replace_all(re, "", "\\0");
    }
    catch (const std::exception&) {
    }

    return 0;
}
