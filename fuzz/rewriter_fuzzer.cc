#include <cstdint>
#include <string>

#include "bishop/error.hpp"
#include "bishop/flags.hpp"
#include "bishop/nfa.hpp"
#include "bishop/regex.hpp"
#include "bishop/rewriter.hpp"

static constexpr size_t kMaxPattern = 256;
static constexpr size_t kMaxRepl = 256;
static constexpr size_t kMaxText = 2048;

static std::string next_field(const std::string& s, size_t& pos, size_t cap)
{
    size_t start = pos;
    while (pos < s.size() && s[pos] != '\n') ++pos;
    std::string field = s.substr(start, std::min(pos - start, cap));
    if (pos < s.size()) ++pos;
    return field;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 3) return 0;

    const uint8_t flag_byte = data[0];
    bishop::Flag flags = bishop::Flag::NONE;
    if (flag_byte & 0x01) flags |= bishop::Flag::IGNORECASE;
    if (flag_byte & 0x02) flags |= bishop::Flag::MULTILINE;
    if (flag_byte & 0x04) flags |= bishop::Flag::DOTALL;

    const std::string body(reinterpret_cast<const char*>(data + 1), size - 1);
    size_t pos = 0;

    const std::string pattern = next_field(body, pos, kMaxPattern);
    const std::string repl = next_field(body, pos, kMaxRepl);
    const std::string text = next_field(body, pos, kMaxText);

    try {
        auto parts = bishop::rewriter::parse_replacement(repl);
        (void)bishop::rewriter::dump_replacement(parts);
    }
    catch (const std::exception&) {
    }

    try {
        bishop::Regex re;
        re.compile(pattern, flags);

        (void)bishop::rewriter::replace_first(re, text, repl);
        (void)bishop::rewriter::replace_all(re, text, repl);

        (void)bishop::rewriter::replace_fn(re, text,
                                           [](const nfa::MatchData& md, const std::string& /*src*/) -> std::string {
                                               return md.matched ? std::string(md.end - md.start, 'X') : "";
                                           });

        (void)bishop::rewriter::replace_first(re, "", repl);
        (void)bishop::rewriter::replace_all(re, "", repl);
    }
    catch (const bishop::RegexError&) {
    }
    catch (const std::exception&) {
    }

    return 0;
}
