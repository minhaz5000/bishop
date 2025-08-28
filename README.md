# Bishop

## A NFA based Regex Engine

This is a simple **regular expression engine** implemented in C++ for the Esmeralda Compiler Lexer module.
It converts a regex into postfix notation, builds a **Non-deterministic Finite Automaton (NFA)**,
and simulates the NFA to check if strings match the regex.

______________________________________________________________________

## 🚀 Features

- Tokenizer for regex input
- Shunting-yard algorithm for infix → postfix
- NFA construction (Thompson's construction)
- NFA simulation with ε-closure
- Supports:
  - Concatenation
  - Alternation (`|`)
  - Kleene star (`*`)
  - Plus (`+`)
  - Optional (`?`)

... More coming soon.

______________________________________________________________________
