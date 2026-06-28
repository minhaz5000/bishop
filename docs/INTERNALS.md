# Bishop Internals

## Architecture Overview

Bishop is a C++17 NFA-based regular expression engine designed to serve as the
lexer backend for the Esmeralda compiler. The pipeline is:

Pattern string

│

▼

tokenizer::tokenize()

│  Produces a flat list of Token structs. Handles escape sequences,

│  character classes, POSIX classes, and special syntax.

▼

tokenizer::strip_anchors()

│  Removes leading ^ and trailing $ tokens; records their presence.

▼

tokenizer::apply_ignorecase()   [if IGNORECASE flag set]

│  Expands Literal and CharClass tokens to cover both cases.

▼

tokenizer::insert_concats()

│  Inserts explicit Concat tokens between adjacent atoms.

▼

tokenizer::to_postfix()

│  Shunting-yard conversion to postfix (RPN) notation.

▼

nfa::parse_postfix()

│  Thompson's construction: builds an NFA from the postfix token stream.

│  Each operator pops NFA fragments from a stack and pushes a new one.

▼

nfa::NFA

│  A graph of State* nodes with character transitions, epsilon edges,

│  and special fields for word boundaries, lookahead, and groups.

▼

nfa::nfa_match()

│  Subset simulation: maintains a set of active states, advancing on

│  each input character, computing epsilon closures, checking DOTALL /

│  word-boundary filters from the Flag parameter.

▼

bool (match result)

## NFA State Structure

```cpp
struct State {
    int  id;
    bool is_accept;
    std::unordered_map<char, State*> transitions;  // char → next state
    std::vector<State*>              epsilon;       // ε-transitions
    State*                           any_transition; // for '.'
    int          group_start;   // capture group open marker (-1 = none)
    int          group_end;     // capture group close marker (-1 = none)
    BoundaryType boundary;      // \b / \B assertion
    LookType     look;          // lookahead type
    NFA*         look_nfa;      // sub-NFA for lookahead predicate
};
```

States are heap-allocated via `new`. There is currently no arena or pool
allocator; each `create_nfa_*` factory allocates exactly the states it needs.

## Thompson's Construction

Each regex operator creates a small NFA fragment:

| Operator | States added | Edges added |
|----------|-------------|-------------|
| Literal `a` | 2 | 1 char transition |
| `.` (dot) | 2 | 1 any_transition |
| `[…]` | 2 | up to 256 char transitions |
| Concat | 0 | 1 epsilon (joins two fragments) |
| Union `|` | 2 | 4 epsilon |
| Kleene `*` | 2 | 4 epsilon |
| Plus `+` | 2 | 3 epsilon |
| Question `?` | 2 | 3 epsilon |
| `{n,m}` | 2×n states | clones inner fragment n times |

## DFA Subset Construction

`dfa::nfa_to_dfa()` implements the standard powerset construction:

1. Start state = ε-closure({nfa.start})
2. For each unprocessed DFA state S and each character c ∈ [0,255]:
   - move(S, c) = union of char/any transitions from each NFA state in S
   - next = ε-closure(move(S, c))
   - If next is new, add it as a DFA state
3. A DFA state is accepting iff any constituent NFA state is accepting

The worst case is exponential (2^n DFA states for n NFA states), but in
practice most patterns produce small DFAs.

## DFA Minimization

`dfa::minimize()` uses iterative partition refinement (a simplified variant of
Hopcroft's algorithm):

1. Initial partition: {accepting} and {non-accepting}
2. Build a "signature" for each state: its current partition class plus the
   class of each transition target for every character 0–255
3. Re-partition states by signature
4. Repeat until stable
5. Build the minimized DFA from the final partition

## Lexer

`bishop::Lexer` implements longest-match (maximal munch) lexing:

- Rules are added with `add_rule(name, pattern)` and compiled to anchored
  patterns `^(?:pattern)` so they only match at the current scan position.
- `scan()` iterates through the input, at each position trying all rules from
  longest possible match down to length 1 (O(n²) per character).
- A "skip" rule discards its match without emitting a token.
- An optional `LexAction` callback transforms the matched value before storage.

## Pattern Analyzer

`bishop::analyzer::analyse()` inspects a pattern's token stream and postfix
form without running the NFA, computing:

- Estimated state count (sum of states that each operator contributes)
- Star height (maximum nesting depth of Kleene/Plus/Question operators)
- Group count and nesting depth
- Catastrophic backtracking flag (star height ≥ 2 with nested union)
- Complexity score (0–100, weighted sum of the above)

## Optimizer

`bishop::optimizer` provides lightweight NFA post-processing passes:

- **Epsilon chain collapse**: if a state has no character transitions and
  exactly one epsilon target, replace it with a direct edge to the target.
- **Dead state removal**: identifies states that cannot reach an accepting
  state and removes them (reporting count only; actual removal TBD).

## Serializer

`bishop::serial` supports two formats for `dfa::DFA`:

- **Binary**: little-endian, magic `0x42495346`, version `0x0001`, followed
  by state records (id, accept bit, transition count, transitions).
- **Text**: ASCII `BISHOP_DFA <version>` header, one state per block,
  transitions as decimal `<char_value> <dst_id>` pairs, terminated by `END`.

## Cache

`bishop::cache::PatternCache` wraps an `LRUCache<string, CompiledEntry>` to
avoid recompiling the same pattern. The global cache is accessed via
`bishop::cache::global_cache()`. Access is thread-safe via `std::mutex`.

## Flags

Flags are `uint32_t` bitmasks composed via `operator|`:

| Flag | Effect |
|------|--------|
| `IGNORECASE` | Expands all literals and charclasses to both cases |
| `MULTILINE` | (tokenized, not yet wired to simulation) |
| `DOTALL` | `.` matches `\n` |
| `EXTENDED` | Whitespace and `#…` comments ignored in pattern |
| `GLOBAL` | (marker for multi-match mode in Regex::find_all) |
