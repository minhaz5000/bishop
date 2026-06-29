#include <cstdint>
#include <string>

#include "bishop/analyzer.hpp"
#include "bishop/error.hpp"
#include "bishop/tokenizer.hpp"

static constexpr size_t kMaxPattern = 1024;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0) return 0;

    const std::string pattern(reinterpret_cast<const char*>(data), std::min(size, kMaxPattern));

    try {
        const auto info = bishop::analyzer::analyse(pattern);
        (void)bishop::analyzer::complexity_score(info);
        (void)bishop::analyzer::pattern_summary(info);
        (void)bishop::analyzer::has_catastrophic_backtracking(pattern);

        auto tokens = tokenizer::tokenize(pattern);
        (void)bishop::analyzer::star_height(tokens);
        (void)bishop::analyzer::nesting_depth(tokens);
        (void)bishop::analyzer::group_count(tokens);

        auto tokens2 = tokenizer::insert_concats(tokens);
        auto postfix = tokenizer::to_postfix(tokens2);
        (void)bishop::analyzer::estimate_state_count(postfix);
    }
    catch (const bishop::RegexError&) {
    }
    catch (const std::exception&) {
    }

    return 0;
}
