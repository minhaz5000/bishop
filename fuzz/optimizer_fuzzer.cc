#include <cstdint>
#include <string>

#include "bishop/error.hpp"
#include "bishop/nfa.hpp"
#include "bishop/optimizer.hpp"
#include "bishop/tokenizer.hpp"

static constexpr size_t kMaxPattern = 512;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0) return 0;

    const std::string pattern(reinterpret_cast<const char*>(data), std::min(size, kMaxPattern));

    try {
        auto tokens = tokenizer::tokenize(pattern);
        tokenizer::strip_anchors(tokens);
        tokens = tokenizer::insert_concats(tokens);
        auto pf = tokenizer::to_postfix(tokens);
        auto n = nfa::parse_postfix(pf);

        const auto pre = bishop::optimizer::nfa_stats(n);
        (void)bishop::optimizer::count_dead_states(n);
        (void)bishop::optimizer::count_epsilon_chains(n);

        bishop::optimizer::remove_dead_states(n);
        bishop::optimizer::collapse_epsilon_chains(n);

        const auto post = bishop::optimizer::full_optimize(n);
        (void)bishop::optimizer::stats_summary(post);

        (void)nfa::nfa_match(n, "");
        (void)nfa::nfa_match(n, "a");
        (void)nfa::nfa_match(n, pattern);
        (void)nfa::nfa_match(n, std::string(32, '\x00'));

        (void)nfa::nfa_match_captures(n, "");
        (void)nfa::nfa_match_captures(n, pattern);
    }
    catch (const bishop::RegexError&) {
    }
    catch (const std::exception&) {
    }

    return 0;
}
