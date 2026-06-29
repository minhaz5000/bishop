#include <cstdint>
#include <sstream>
#include <string>

#include "bishop/dfa.hpp"
#include "bishop/error.hpp"
#include "bishop/nfa.hpp"
#include "bishop/optimizer.hpp"
#include "bishop/serializer.hpp"
#include "bishop/tokenizer.hpp"

static constexpr size_t kMaxPattern = 512;
static constexpr size_t kMaxText = 2048;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 2) return 0;

    size_t sep = 0;
    while (sep < size && data[sep] != '\n') ++sep;

    const size_t pat_len = std::min(sep, kMaxPattern);
    const size_t text_off = (sep < size) ? sep + 1 : sep;
    const size_t text_len = std::min(size - text_off, kMaxText);

    const std::string pattern(reinterpret_cast<const char*>(data), pat_len);
    const std::string text(reinterpret_cast<const char*>(data + text_off), text_len);
    const std::string probe = text.substr(0, std::min(text_len, size_t{64}));

    try {
        auto tokens = tokenizer::tokenize(pattern);
        tokenizer::strip_anchors(tokens);
        tokens = tokenizer::insert_concats(tokens);
        auto pf = tokenizer::to_postfix(tokens);
        auto n = nfa::parse_postfix(pf);

        (void)bishop::optimizer::nfa_stats(n);
        (void)bishop::optimizer::count_dead_states(n);
        (void)bishop::optimizer::count_epsilon_chains(n);
        bishop::optimizer::remove_dead_states(n);
        bishop::optimizer::collapse_epsilon_chains(n);
        auto full_stats = bishop::optimizer::full_optimize(n);
        (void)bishop::optimizer::stats_summary(full_stats);

        auto d = dfa::nfa_to_dfa(n);
        auto m = dfa::minimize(d);

        const bool match_d = dfa::dfa_match(d, text);
        const bool match_m = dfa::dfa_match(m, text);

        if (d.valid && m.valid && match_d != match_m) {
            __builtin_trap();
        }

        if (d.valid) {
            std::ostringstream oss;
            bishop::serial::serialize_dfa(d, oss);
            std::istringstream iss(oss.str());
            auto d2 = bishop::serial::deserialize_dfa(iss);
            if (d2.valid) {
                const bool match_d2 = dfa::dfa_match(d2, text);
                if (match_d != match_d2) __builtin_trap();
            }
        }

        if (m.valid) {
            std::ostringstream oss;
            bishop::serial::serialize_dfa_text(m, oss);
            std::istringstream iss(oss.str());
            auto m2 = bishop::serial::deserialize_dfa_text(iss);
            if (m2.valid) {
                const bool match_m2 = dfa::dfa_match(m2, text);
                if (match_m != match_m2) __builtin_trap();
            }
        }

        if (d.valid) dfa::dfa_match(d, "");
        if (m.valid) dfa::dfa_match(m, "");
    }
    catch (const bishop::RegexError&) {
    }
    catch (const std::exception&) {
    }

    return 0;
}
