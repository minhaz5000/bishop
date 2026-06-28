#include <benchmark/benchmark.h>

#include <string>

#include "bishop/nfa.hpp"
#include "bishop/tokenizer.hpp"

using nfa::NFA;
using nfa::nfa_match;
using nfa::parse_postfix;

static NFA build_regex(const std::string& pattern)
{
    auto tokens = tokenizer::to_postfix(tokenizer::insert_concats(tokenizer::tokenize(pattern)));
    return parse_postfix(tokens);
}

static void run_match(benchmark::State& state, const std::string& pattern, const std::string& text)
{
    NFA re = build_regex(pattern);
    for (auto _ : state) {
        bool ans = nfa_match(re, text);
        benchmark::DoNotOptimize(ans);
    }
}
static void BM_NFA_DeepNesting_1(benchmark::State& state)
{
    std::string pattern = std::string(1, '(') + "a" + std::string(1, ')') + "+";
    std::string text(1 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_1);

static void BM_NFA_Catastrophic_Simulation_1(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(1 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_1);

static void BM_NFA_DeepNesting_2(benchmark::State& state)
{
    std::string pattern = std::string(2, '(') + "a" + std::string(2, ')') + "+";
    std::string text(2 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_2);

static void BM_NFA_Catastrophic_Simulation_2(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(2 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_2);

static void BM_NFA_DeepNesting_3(benchmark::State& state)
{
    std::string pattern = std::string(3, '(') + "a" + std::string(3, ')') + "+";
    std::string text(3 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_3);

static void BM_NFA_Catastrophic_Simulation_3(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(3 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_3);

static void BM_NFA_DeepNesting_4(benchmark::State& state)
{
    std::string pattern = std::string(4, '(') + "a" + std::string(4, ')') + "+";
    std::string text(4 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_4);

static void BM_NFA_Catastrophic_Simulation_4(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(4 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_4);

static void BM_NFA_DeepNesting_5(benchmark::State& state)
{
    std::string pattern = std::string(5, '(') + "a" + std::string(5, ')') + "+";
    std::string text(5 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_5);

static void BM_NFA_Catastrophic_Simulation_5(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(5 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_5);

static void BM_NFA_DeepNesting_6(benchmark::State& state)
{
    std::string pattern = std::string(6, '(') + "a" + std::string(6, ')') + "+";
    std::string text(6 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_6);

static void BM_NFA_Catastrophic_Simulation_6(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(6 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_6);

static void BM_NFA_DeepNesting_7(benchmark::State& state)
{
    std::string pattern = std::string(7, '(') + "a" + std::string(7, ')') + "+";
    std::string text(7 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_7);

static void BM_NFA_Catastrophic_Simulation_7(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(7 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_7);

static void BM_NFA_DeepNesting_8(benchmark::State& state)
{
    std::string pattern = std::string(8, '(') + "a" + std::string(8, ')') + "+";
    std::string text(8 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_8);

static void BM_NFA_Catastrophic_Simulation_8(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(8 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_8);

static void BM_NFA_DeepNesting_9(benchmark::State& state)
{
    std::string pattern = std::string(9, '(') + "a" + std::string(9, ')') + "+";
    std::string text(9 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_9);

static void BM_NFA_Catastrophic_Simulation_9(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(9 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_9);

static void BM_NFA_DeepNesting_10(benchmark::State& state)
{
    std::string pattern = std::string(10, '(') + "a" + std::string(10, ')') + "+";
    std::string text(10 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_10);

static void BM_NFA_Catastrophic_Simulation_10(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(10 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_10);

static void BM_NFA_DeepNesting_11(benchmark::State& state)
{
    std::string pattern = std::string(11, '(') + "a" + std::string(11, ')') + "+";
    std::string text(11 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_11);

static void BM_NFA_Catastrophic_Simulation_11(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(11 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_11);

static void BM_NFA_DeepNesting_12(benchmark::State& state)
{
    std::string pattern = std::string(12, '(') + "a" + std::string(12, ')') + "+";
    std::string text(12 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_12);

static void BM_NFA_Catastrophic_Simulation_12(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(12 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_12);

static void BM_NFA_DeepNesting_13(benchmark::State& state)
{
    std::string pattern = std::string(13, '(') + "a" + std::string(13, ')') + "+";
    std::string text(13 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_13);

static void BM_NFA_Catastrophic_Simulation_13(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(13 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_13);

static void BM_NFA_DeepNesting_14(benchmark::State& state)
{
    std::string pattern = std::string(14, '(') + "a" + std::string(14, ')') + "+";
    std::string text(14 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_14);

static void BM_NFA_Catastrophic_Simulation_14(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(14 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_14);

static void BM_NFA_DeepNesting_15(benchmark::State& state)
{
    std::string pattern = std::string(15, '(') + "a" + std::string(15, ')') + "+";
    std::string text(15 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_15);

static void BM_NFA_Catastrophic_Simulation_15(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(15 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_15);

static void BM_NFA_DeepNesting_16(benchmark::State& state)
{
    std::string pattern = std::string(16, '(') + "a" + std::string(16, ')') + "+";
    std::string text(16 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_16);

static void BM_NFA_Catastrophic_Simulation_16(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(16 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_16);

static void BM_NFA_DeepNesting_17(benchmark::State& state)
{
    std::string pattern = std::string(17, '(') + "a" + std::string(17, ')') + "+";
    std::string text(17 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_17);

static void BM_NFA_Catastrophic_Simulation_17(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(17 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_17);

static void BM_NFA_DeepNesting_18(benchmark::State& state)
{
    std::string pattern = std::string(18, '(') + "a" + std::string(18, ')') + "+";
    std::string text(18 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_18);

static void BM_NFA_Catastrophic_Simulation_18(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(18 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_18);

static void BM_NFA_DeepNesting_19(benchmark::State& state)
{
    std::string pattern = std::string(19, '(') + "a" + std::string(19, ')') + "+";
    std::string text(19 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_19);

static void BM_NFA_Catastrophic_Simulation_19(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(19 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_19);

static void BM_NFA_DeepNesting_20(benchmark::State& state)
{
    std::string pattern = std::string(20, '(') + "a" + std::string(20, ')') + "+";
    std::string text(20 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_20);

static void BM_NFA_Catastrophic_Simulation_20(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(20 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_20);

static void BM_NFA_DeepNesting_21(benchmark::State& state)
{
    std::string pattern = std::string(21, '(') + "a" + std::string(21, ')') + "+";
    std::string text(21 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_21);

static void BM_NFA_Catastrophic_Simulation_21(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(21 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_21);

static void BM_NFA_DeepNesting_22(benchmark::State& state)
{
    std::string pattern = std::string(22, '(') + "a" + std::string(22, ')') + "+";
    std::string text(22 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_22);

static void BM_NFA_Catastrophic_Simulation_22(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(22 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_22);

static void BM_NFA_DeepNesting_23(benchmark::State& state)
{
    std::string pattern = std::string(23, '(') + "a" + std::string(23, ')') + "+";
    std::string text(23 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_23);

static void BM_NFA_Catastrophic_Simulation_23(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(23 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_23);

static void BM_NFA_DeepNesting_24(benchmark::State& state)
{
    std::string pattern = std::string(24, '(') + "a" + std::string(24, ')') + "+";
    std::string text(24 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_24);

static void BM_NFA_Catastrophic_Simulation_24(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(24 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_24);

static void BM_NFA_DeepNesting_25(benchmark::State& state)
{
    std::string pattern = std::string(25, '(') + "a" + std::string(25, ')') + "+";
    std::string text(25 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_25);

static void BM_NFA_Catastrophic_Simulation_25(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(25 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_25);

static void BM_NFA_DeepNesting_26(benchmark::State& state)
{
    std::string pattern = std::string(26, '(') + "a" + std::string(26, ')') + "+";
    std::string text(26 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_26);

static void BM_NFA_Catastrophic_Simulation_26(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(26 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_26);

static void BM_NFA_DeepNesting_27(benchmark::State& state)
{
    std::string pattern = std::string(27, '(') + "a" + std::string(27, ')') + "+";
    std::string text(27 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_27);

static void BM_NFA_Catastrophic_Simulation_27(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(27 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_27);

static void BM_NFA_DeepNesting_28(benchmark::State& state)
{
    std::string pattern = std::string(28, '(') + "a" + std::string(28, ')') + "+";
    std::string text(28 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_28);

static void BM_NFA_Catastrophic_Simulation_28(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(28 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_28);

static void BM_NFA_DeepNesting_29(benchmark::State& state)
{
    std::string pattern = std::string(29, '(') + "a" + std::string(29, ')') + "+";
    std::string text(29 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_29);

static void BM_NFA_Catastrophic_Simulation_29(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(29 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_29);

static void BM_NFA_DeepNesting_30(benchmark::State& state)
{
    std::string pattern = std::string(30, '(') + "a" + std::string(30, ')') + "+";
    std::string text(30 * 5, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_DeepNesting_30);

static void BM_NFA_Catastrophic_Simulation_30(benchmark::State& state)
{
    std::string pattern = "a*a*a*a*a*a*a*a*a*a*b";
    std::string text(30 * 10, 'a');
    run_match(state, pattern, text);
}
BENCHMARK(BM_NFA_Catastrophic_Simulation_30);

BENCHMARK_MAIN();
