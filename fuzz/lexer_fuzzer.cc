#include <cctype>
#include <cstdint>
#include <string>
#include <vector>

#include "bishop/error.hpp"
#include "bishop/flags.hpp"
#include "bishop/lexer.hpp"

static constexpr size_t kMaxText = 2048;
static constexpr size_t kMaxRuleLen = 128;

static bool is_valid_identifier(const std::string& s)
{
    if (s.empty()) return false;
    for (unsigned char c : s)
        if (!std::isalnum(c) && c != '_') return false;
    return true;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 4) return 0;

    const int max_rules = (data[0] % 8) + 1;
    const uint8_t flag_byte = data[1];
    bishop::Flag rule_flags = bishop::Flag::NONE;
    if (flag_byte & 0x01) rule_flags |= bishop::Flag::IGNORECASE;
    if (flag_byte & 0x02) rule_flags |= bishop::Flag::MULTILINE;

    const char* body = reinterpret_cast<const char*>(data + 2);
    const size_t bsize = size - 2;

    size_t sep = bsize;
    for (size_t i = 0; i < bsize; ++i) {
        if (static_cast<unsigned char>(body[i]) == 0x01) {
            sep = i;
            break;
        }
    }

    const std::string rules_block(body, sep);
    const size_t text_start = (sep < bsize) ? sep + 1 : bsize;
    const std::string input_text(body + text_start, std::min(bsize - text_start, kMaxText));

    try {
        bishop::Lexer lex;
        lex.skip("[ \t\r\n]+");

        int added = 0;
        size_t rpos = 0;
        while (rpos <= rules_block.size() && added < max_rules) {
            size_t nl = rules_block.find('\n', rpos);
            if (nl == std::string::npos) nl = rules_block.size();
            if (nl == rpos) {
                rpos = nl + 1;
                continue;
            }

            std::string line = rules_block.substr(rpos, nl - rpos);
            rpos = nl + 1;

            size_t eq = line.find('=');
            if (eq == std::string::npos || eq == 0 || eq + 1 >= line.size()) continue;

            std::string name = line.substr(0, eq);
            std::string pattern = line.substr(eq + 1, kMaxRuleLen);

            if (!is_valid_identifier(name)) continue;
            if (pattern.empty()) continue;

            try {
                lex.add_rule(name, pattern, rule_flags);
                ++added;
            }
            catch (const bishop::RegexError&) {
            }
        }

        if (added == 0) {
            lex.add_rule("WORD", "[a-zA-Z_][a-zA-Z0-9_]*");
            lex.add_rule("INT", "[0-9]+");
            lex.add_rule("FLOAT", "[0-9]+\\.[0-9]*");
            lex.add_rule("OP", "[+\\-*/=<>!&|^~%]");
            lex.add_rule("PUNCT", "[^a-zA-Z0-9_ \\t\\r\\n]");
        }

        auto tokens = lex.scan(input_text);
        (void)lex.last_tokens().size();

        auto tokens2 = lex.tokenize(input_text);
        (void)tokens2.size();

        for (const auto& tok : tokens) {
            (void)tok.type;
            (void)tok.value;
            (void)tok.line;
            (void)tok.col;
        }

        if (added > 0) {
            lex.has_rule("WORD");
        }
    }
    catch (const bishop::LexError&) {
    }
    catch (const bishop::RegexError&) {
    }
    catch (const std::exception&) {
    }

    return 0;
}
