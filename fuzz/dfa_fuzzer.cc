#include <cstdint>
#include <string>
#include "bishop/dfa.hpp"
#include "bishop/nfa.hpp"
#include "bishop/regex.hpp"
#include "bishop/tokenizer.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 2) return 0;
    size_t sep = 0;
    while (sep < size && data[sep] != '\n') ++sep;

    std::string pattern(reinterpret_cast<const char*>(data), sep);
    std::string text(reinterpret_cast<const char*>(data + (sep < size ? sep + 1 : sep)),
                     sep < size ? size - sep - 1 : 0);
    try {
        auto tokens  = tokenizer::tokenize(pattern);
        tokenizer::strip_anchors(tokens);
        tokens       = tokenizer::insert_concats(tokens);
        auto postfix = tokenizer::to_postfix(tokens);
        auto n       = nfa::parse_postfix(postfix);
        auto d       = dfa::nfa_to_dfa(n);
        auto m       = dfa::minimize(d);
        dfa::dfa_match(d, text);
        dfa::dfa_match(m, text);
    } catch (const bishop::RegexError&) {}
    return 0;
}
