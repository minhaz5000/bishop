# Changelog

## [Unreleased]

### Added
- Bounded repetition quantifiers `{n}`, `{n,m}`, `{n,}`
- Dot wildcard `.` matching any character
- Character classes `[a-z]`, `[^abc]` with range support
- Escape shorthands `\d`, `\w`, `\s`, `\D`, `\W`, `\S`
- Control char escapes `\n`, `\t`, `\r`, `\v`
- Anchor tokens `^` and `$`
- `Regex::search()` for substring matching
- `Regex::find_all()` for all non-overlapping matches
- `Regex::replace()` and `Regex::replace_all()`
- CMake build system
- Unit test framework and initial test suites
- Utility helpers in `bishop::utils`

## [0.1.0]

### Added
- NFA-based regex engine (Thompson's construction)
- Tokenizer with Shunting-yard postfix conversion
- Operators: concatenation, alternation `|`, Kleene star `*`, `+`, `?`
- ClusterFuzzLite integration
- Fuzz corpus seeds
