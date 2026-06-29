#include <cctype>
#include <cstdint>
#include <string>
#include <vector>

#include "bishop/error.hpp"
#include "bishop/lexer.hpp"
#include "bishop/token_stream.hpp"

static constexpr size_t kMaxText = 1024;
static constexpr size_t kMaxRuleLen = 128;

static bool is_valid_ident(const std::string& s)
{
    if (s.empty()) return false;
    for (unsigned char c : s)
        if (!std::isalnum(c) && c != '_') return false;
    return true;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 5) return 0;

    const size_t max_ops = (data[0] % 64) + 1;
    const uint8_t op_seed = data[1];

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

    bishop::Lexer lex;
    lex.skip("[ \t\r\n]+");

    int added = 0;
    size_t rpos = 0;
    while (rpos <= rules_block.size() && added < 6) {
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
        if (!is_valid_ident(name) || pattern.empty()) continue;
        try {
            lex.add_rule(name, pattern);
            ++added;
        }
        catch (const bishop::RegexError&) {
        }
    }
    if (added == 0) {
        lex.add_rule("WORD", "[a-zA-Z_][a-zA-Z0-9_]*");
        lex.add_rule("NUM", "[0-9]+");
        lex.add_rule("PUNCT", "[^a-zA-Z0-9_ \\t\\r\\n]");
    }

    std::vector<bishop::LexToken> tokens;
    try {
        tokens = lex.scan(input_text);
    }
    catch (const bishop::LexError&) {
        return 0;
    }
    catch (const bishop::RegexError&) {
        return 0;
    }
    catch (const std::exception&) {
        return 0;
    }

    if (tokens.empty()) return 0;

    try {
        bishop::TokenStream ts(tokens);

        (void)ts.size();
        (void)ts.remaining();

        uint8_t op = op_seed;
        for (size_t i = 0; i < max_ops && !ts.eof(); ++i, op = static_cast<uint8_t>(op * 31 + 7)) {
            switch (op % 10) {
                case 0:
                    (void)ts.peek();
                    break;
                case 1:
                    (void)ts.peek_at(op % 4);
                    break;
                case 2:
                    (void)ts.peek_is("NUM");
                    (void)ts.peek_is("WORD");
                    break;
                case 3:
                    if (!ts.eof()) (void)ts.consume();
                    break;
                case 4:
                    (void)ts.try_consume("NUM");
                    break;
                case 5:
                    ts.skip(1);
                    break;
                case 6:
                    ts.skip_while([](const bishop::LexToken& t) { return t.type == "NUM"; });
                    break;
                case 7: {
                    ts.save();
                    if (!ts.eof()) (void)ts.consume();
                    ts.restore();
                    break;
                }
                case 8: {
                    ts.save();
                    if (!ts.eof()) (void)ts.consume();
                    ts.commit();
                    break;
                }
                case 9:
                    if (!ts.eof()) {
                        const std::string end_type = ts.peek().type;
                        (void)ts.collect_until(end_type);
                    }
                    break;
            }
        }

        bishop::TokenStream ts2(tokens);
        (void)ts2.collect_while([](const bishop::LexToken& t) { return t.type != "PUNCT"; });

        if (tokens.size() >= 2) {
            bishop::TokenStream ts3(tokens);
            (void)ts3.slice(0, tokens.size() / 2);
        }

        bishop::TokenStream ts4(tokens);
        ts4.skip_all();
        ts4.expect_eof();
    }
    catch (const bishop::TokenStreamError&) {
    }
    catch (const bishop::RegexError&) {
    }
    catch (const std::exception&) {
    }

    return 0;
}
