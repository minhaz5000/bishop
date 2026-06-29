#include <cstdint>
#include <string>

#include "bishop/error.hpp"
#include "bishop/flags.hpp"
#include "bishop/match_iterator.hpp"
#include "bishop/nfa.hpp"
#include "bishop/regex.hpp"

static constexpr size_t kMaxPattern = 512;
static constexpr size_t kMaxText = 4096;
static constexpr size_t kMaxIter = 64;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 3) return 0;

    const uint8_t flag_byte = data[0];
    bishop::Flag flags = bishop::Flag::NONE;
    if (flag_byte & 0x01) flags |= bishop::Flag::IGNORECASE;
    if (flag_byte & 0x02) flags |= bishop::Flag::MULTILINE;
    if (flag_byte & 0x04) flags |= bishop::Flag::DOTALL;
    if (flag_byte & 0x08) flags |= bishop::Flag::EXTENDED;
    if (flag_byte & 0x10) flags |= bishop::Flag::GLOBAL;

    const size_t body_size = size - 2;
    const uint8_t sep_byte = data[1];
    const size_t sep = static_cast<size_t>(sep_byte) % (body_size + 1);

    const char* body = reinterpret_cast<const char*>(data + 2);
    const size_t pat_len = std::min(sep, kMaxPattern);
    const size_t text_off = sep;
    const size_t text_len = std::min(body_size > sep ? body_size - sep : size_t{0}, kMaxText);

    const size_t repl_len = std::min(pat_len, size_t{64});

    const std::string pattern(body, pat_len);
    const std::string text(body + text_off, text_len);
    const std::string repl(body, repl_len);

    try {
        bishop::Regex re;
        re.compile(pattern, flags);

        (void)re.match(text);
        auto [s, e] = re.search(text);
        (void)s;
        (void)e;

        (void)re.find_all(text);

        (void)re.replace(text, repl);
        (void)re.replace_all(text, repl);

        auto md = re.search_match(text);
        if (md.matched) {
            for (size_t i = 0; i < md.group_count(); ++i) (void)md.group_matched(i);
        }
        auto all_md = re.find_all_matches(text);
        (void)all_md.size();

        bishop::MatchIterator it(&re, text);
        bishop::MatchIterator end_it;
        size_t iter_guard = 0;
        for (; it != end_it && iter_guard < kMaxIter; ++it, ++iter_guard) {
            (void)(*it).str();
            (void)(*it).length();
        }

        (void)re.match("");
        (void)re.search("");
        (void)re.find_all("");
        (void)re.search_match("");

        (void)re.pattern();
        (void)re.flags();
        (void)re.anchored_start();
        (void)re.anchored_end();
    }
    catch (const bishop::RegexError&) {
    }
    catch (const std::exception&) {
    }

    return 0;
}
