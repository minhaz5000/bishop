#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "bishop/cache.hpp"
#include "bishop/dfa.hpp"
#include "bishop/error.hpp"
#include "bishop/flags.hpp"
#include "bishop/nfa.hpp"
#include "bishop/tokenizer.hpp"

static constexpr size_t kMaxPatterns = 16;
static constexpr size_t kMaxPat = 128;

static bool compile_nfa(const std::string& pattern, bishop::Flag flags, nfa::NFA& out)
{
    try {
        auto tokens = tokenizer::tokenize(pattern, flags);
        tokenizer::strip_anchors(tokens);
        tokens = tokenizer::insert_concats(tokens);
        auto pf = tokenizer::to_postfix(tokens);
        out = nfa::parse_postfix(pf);
        return true;
    }
    catch (...) {
        return false;
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 3) return 0;

    const size_t cap = static_cast<size_t>(data[0] % 8) + 1;
    const uint8_t flag_byte = data[1];
    bishop::Flag flags = bishop::Flag::NONE;
    if (flag_byte & 0x01) flags |= bishop::Flag::IGNORECASE;
    if (flag_byte & 0x02) flags |= bishop::Flag::MULTILINE;

    const char* body = reinterpret_cast<const char*>(data + 2);
    const size_t bsize = size - 2;

    std::vector<std::string> patterns;
    size_t pos = 0;
    while (pos < bsize && patterns.size() < kMaxPatterns) {
        size_t nl = bsize;
        for (size_t i = pos; i < bsize; ++i) {
            if (body[i] == '\n') {
                nl = i;
                break;
            }
        }
        std::string pat(body + pos, std::min(nl - pos, kMaxPat));
        pos = (nl < bsize) ? nl + 1 : bsize;
        if (!pat.empty()) patterns.push_back(std::move(pat));
    }

    if (patterns.empty()) return 0;

    bishop::cache::PatternCache cache(cap);

    for (const auto& p : patterns) {
        nfa::NFA n{nullptr, nullptr};
        if (!compile_nfa(p, flags, n)) continue;
        cache.put_nfa(p, flags, std::move(n));
        nfa::NFA out{nullptr, nullptr};
        (void)cache.get_nfa(p, flags, out);
    }

    for (const auto& p : patterns) {
        nfa::NFA n{nullptr, nullptr};
        if (!compile_nfa(p, flags, n)) continue;
        try {
            auto d = dfa::nfa_to_dfa(n);
            cache.put_dfa(p, flags, d);
            dfa::DFA out;
            (void)cache.get_dfa(p, flags, out);
        }
        catch (...) {
        }
    }

    (void)cache.size();
    (void)cache.capacity();
    (void)cache.hits();
    (void)cache.misses();
    (void)cache.hit_rate();

    cache.resize(1);
    (void)cache.size();
    cache.resize(cap * 2);

    cache.clear();
    (void)cache.size();

    auto& gc = bishop::cache::global_cache();
    for (const auto& p : patterns) {
        nfa::NFA n{nullptr, nullptr};
        if (!compile_nfa(p, flags, n)) continue;
        gc.put_nfa(p, flags, std::move(n));
        nfa::NFA out{nullptr, nullptr};
        (void)gc.get_nfa(p, flags, out);
    }

    return 0;
}
