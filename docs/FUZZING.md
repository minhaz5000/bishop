# Fuzzing Bishop

## Overview

Bishop ships ClusterFuzzLite configuration targeting three harnesses:

| Harness | Source | Target |
|---------|--------|--------|
| `regex_fuzzer` | `fuzz/regex_fuzzer.cc` | Full `Regex::compile` + `match` + `search` pipeline |
| `dfa_fuzzer` | `fuzz/dfa_fuzzer.cc` | NFA-to-DFA conversion and minimization |
| `lexer_fuzzer` | `fuzz/lexer_fuzzer.cc` | `Lexer::scan` with multiple rule types |

All harnesses are compiled with `-fsanitize=address,undefined` (see
`.clusterfuzzlite/project.yaml`).

## Input Format

### regex_fuzzer and dfa_fuzzer

The input is split on the first `\n` byte:
<pattern>\n<text>

The pattern is compiled with `Regex::compile`. If compilation throws
`bishop::RegexError`, the harness returns 0 (expected). Memory-safety crashes
(segfault, ASan abort) are reported as crashes.

### lexer_fuzzer

The input is split on the first `\0` byte:
<text>\0<rules>

`<rules>` is a newline-separated list of `NAME=PATTERN` pairs. The harness
builds a `Lexer`, adds each rule, then calls `scan(<text>)`. `LexError` is
caught; crashes are reported.

## Building Locally

```bash
# AddressSanitizer build
CXX=clang++ CXXFLAGS="-fsanitize=address -g -O1" \
    ./.clusterfuzzlite/build.sh

# Run regex fuzzer
./out/regex_fuzzer fuzz/corpus/ -runs=10000
```

## Seed Corpus

Seeds live in `fuzz/corpus/` and cover:

- Simple literals and operators (`a`, `a|b`, `a*b`)
- Character classes (`[a-z]+`, `[^0-9]`)
- Escape shorthands (`\d+`, `\w+`)
- Bounded repetition (`a{3}`, `[a-z]{2}`, `a{1,4}`)
- Groups (`(ab){2}`)
- Anchors (`^abc`, `abc$`)
- Search patterns with newline-separated text

## Known Crash Classes

The following crash classes are expected to be found by fuzzing. Each
represents a genuine memory-safety bug in the implementation:

### 1. Stack underflow in `parse_postfix`

**Trigger**: bare quantifier with no preceding atom, e.g. pattern `*`, `|`, or `a|`

**Root cause**: `parse_postfix` calls `st.top()` / `st.pop()` on unary and
binary operators without checking that the stack is non-empty.

**Sanitizer**: ASan stack-use-after-scope or abort on empty container.

### 2. Stack-buffer-overflow in `create_nfa_charclass`

**Trigger**: a negated character class with few excluded characters, e.g. `[^a]`

**Root cause**: `create_nfa_charclass` copies matching characters into a
`char buf[128]` on the stack. A negated class like `[^a]` has 255 matching
characters, writing 127 bytes past the end of the buffer.

**Sanitizer**: ASan `stack-buffer-overflow`.

### 3. Null-pointer dereference via broken NFA clone

**Trigger**: any Dot wildcard inside a bounded repeat with n ≥ 2, e.g. `.{2}`

**Root cause**: `collect_states` (used by `clone_nfa`) follows epsilon and
character transitions but silently skips `any_transition`. The Dot NFA's
accept state is therefore absent from the clone's state map, so
`mapping[src.end]` returns `nullptr`. The second `create_nfa_concat` call
dereferences that null end pointer.

**Sanitizer**: ASan / segfault.

### 4. Stack-buffer-overflow in `Lexer::copy_lexeme`

**Trigger**: a lexer rule that matches a long token (> 512 bytes), e.g.
rule `[a-z]+` with 600 lowercase letters as input.

**Root cause**: `copy_lexeme` in `src/lexer.cpp` copies the matched lexeme
into a 512-byte stack buffer (`char scratch[kScratchBufLen]`) using
`std::memcpy` without validating that `len ≤ kScratchBufLen`.

**Sanitizer**: ASan `stack-buffer-overflow`.

## Adding New Harnesses

Harnesses must:

1. Call `LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)`.
2. Catch `bishop::RegexError` and `bishop::LexError`.
3. Return 0 for all non-crash paths.
4. Be listed in `.clusterfuzzlite/project.yaml` under `fuzz_targets`.
5. Be compiled in `.clusterfuzzlite/build.sh` with `$LIB_FUZZING_ENGINE`.

## Corpus Management

Keep seeds small (< 200 bytes) and structurally diverse. Per-harness
subdirectories are supported by ClusterFuzzLite — place lexer seeds in
`fuzz/corpus_lexer/` if needed.
