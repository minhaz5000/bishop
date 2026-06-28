#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "bishop/error.hpp"
#include "bishop/lexer.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 3) return 0;

    size_t sep = 0;
    while (sep < size && data[sep] != '\0') ++sep;

    std::string text(reinterpret_cast<const char*>(data), sep);
    std::string rules_str(
        reinterpret_cast<const char*>(data + (sep < size ? sep + 1 : sep)),
        sep < size ? size - sep - 1 : 0);

    try {
        bishop::Lexer lex;
        lex.skip("[ \t\n\r]+");

        size_t rpos = 0;
        int    added = 0;
        while (rpos < rules_str.size() && added < 8) {
            size_t nl = rules_str.find('\n', rpos);
            if (nl == std::string::npos) nl = rules_str.size();
            std::string line = rules_str.substr(rpos, nl - rpos);
            rpos = nl + 1;

            size_t eq = line.find('=');
            if (eq == std::string::npos || eq == 0) continue;
            std::string name    = line.substr(0, eq);
            std::string pattern = line.substr(eq + 1);
            if (pattern.empty()) continue;
            try {
                lex.add_rule(name, pattern);
                ++added;
            } catch (const bishop::RegexError&) {}
        }

        if (added == 0) {
            lex.add_rule("WORD",  "[a-zA-Z_]\\w*");
            lex.add_rule("NUM",   "\\d+");
            lex.add_rule("PUNCT", "[^a-zA-Z0-9 \t\n]");
        }

        lex.scan(text);
    } catch (const bishop::RegexError&) {
    } catch (const bishop::LexError&) {
    }

    return 0;
}
