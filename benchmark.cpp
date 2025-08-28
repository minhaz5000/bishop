#include <benchmark/benchmark.h>
#include "nfa.hpp"
#include "tokenizer.hpp"

using nfa::NFA;
using nfa::parse_postfix;
using nfa::nfa_match;

static NFA build_regex(const std::string& pattern) {
    auto tokens = tokenizer::to_postfix(
        tokenizer::insert_concats(
            tokenizer::tokenize(pattern)
        )
    );
    return parse_postfix(tokens);
}

static void run_match(benchmark::State& state, const std::string& pattern, const std::string& text) {
    NFA re = build_regex(pattern);
    for (auto _ : state) {
        bool ans = nfa_match(re, text);
        benchmark::DoNotOptimize(ans);
    }
}

// ---------------------- BENCHMARKS ----------------------

// Simple literal match
static void BM_NFA_Simple(benchmark::State& state) {
    run_match(state, "abc", "abc");
}

// Alternation
static void BM_NFA_Alternation(benchmark::State& state) {
    run_match(state, "a|b|c", "b");
}

// Kleene star repetition
static void BM_NFA_KleeneStar(benchmark::State& state) {
    run_match(state, "a*", "aaaaaaaaaaaaaaaaaaaaaaaa");
}

// Concatenation with star
static void BM_NFA_ConcatStar(benchmark::State& state) {
    run_match(state, "ab*c", "abbbbbbbbbbbbbbbc");
}

// Nested groups with plus
static void BM_NFA_NestedPlus(benchmark::State& state) {
    run_match(state, "a(bb)+a*", "abbbbaaaa");
}

// Long pattern
static void BM_NFA_LongPattern(benchmark::State& state) {
    run_match(state, "((ab|cd)*efg)+h", "ababcdabefgh");
}

// Stress test with very long text
static void BM_NFA_LongText(benchmark::State& state) {
    std::string long_text(1000, 'a');
    run_match(state, "a*b", long_text + "b");
}

BENCHMARK(BM_NFA_Simple);
BENCHMARK(BM_NFA_Alternation);
BENCHMARK(BM_NFA_KleeneStar);
BENCHMARK(BM_NFA_ConcatStar);
BENCHMARK(BM_NFA_NestedPlus);
BENCHMARK(BM_NFA_LongPattern);
BENCHMARK(BM_NFA_LongText);

BENCHMARK_MAIN();
