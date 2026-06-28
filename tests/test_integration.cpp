#include "bishop/analyzer.hpp"
#include "bishop/cache.hpp"
#include "bishop/lexer.hpp"
#include "bishop/regex.hpp"
#include "bishop/rewriter.hpp"
#include "bishop/splitter.hpp"
#include "bishop/token_stream.hpp"
#include "framework.hpp"

using namespace bishop;

static std::vector<std::vector<std::string>> parse_csv(const std::string& text)
{
    Lexer lex;
    lex.add_rule("FIELD", "[^,\\n]+");
    lex.add_rule("EMPTY", "(?=,|\\n)");
    lex.add_rule("COMMA", ",");
    lex.add_rule("NEWLINE", "\\n");

    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> current_row;

    auto tokens = lex.scan(text);
    TokenStream ts(std::move(tokens));

    while (!ts.eof()) {
        if (ts.peek_is("NEWLINE")) {
            ts.consume();
            rows.push_back(current_row);
            current_row.clear();
        } else if (ts.peek_is("COMMA")) {
            ts.consume();
        } else {
            current_row.push_back(ts.consume().value);
        }
    }
    if (!current_row.empty()) rows.push_back(current_row);

    return rows;
}

TEST(integration_csv_lexer_token_stream)
{
    auto rows = parse_csv("a,b,c\n1,2,3\n");
    ASSERT_EQ(rows.size(), 2u);
    ASSERT_EQ(rows[0][0], "a");
    ASSERT_EQ(rows[0][2], "c");
    ASSERT_EQ(rows[1][1], "2");
}

TEST(integration_rewrite_phone_numbers)
{
    Regex r("[0-9][0-9][0-9]-[0-9][0-9][0-9][0-9]");
    std::string result = rewriter::replace_all(r, "Call 555-1234 or 555-5678 today", "[\\0]");
    ASSERT_NE(result.find("[555-1234]"), std::string::npos);
    ASSERT_NE(result.find("[555-5678]"), std::string::npos);
}

TEST(integration_log_line_split_and_rewrite)
{
    std::string log = "INFO:startup\nWARN:low memory\nERROR:disk full";
    auto lines = splitter::split(log, "\\n");
    ASSERT_EQ(lines.size(), 3u);

    Regex level_re("[A-Z]+");
    std::vector<std::string> levels;
    for (auto& line : lines) {
        auto md = level_re.search_match(line);
        if (md.matched) levels.push_back(line.substr(md.start, md.end - md.start));
    }
    ASSERT_EQ(levels[0], "INFO");
    ASSERT_EQ(levels[1], "WARN");
    ASSERT_EQ(levels[2], "ERROR");
}

TEST(integration_expression_tokeniser)
{
    Lexer lex;
    lex.add_rule("NUM", "[0-9]+");
    lex.add_rule("OP", "[+\\-*/]");
    lex.add_rule("LPAREN", "\\(");
    lex.add_rule("RPAREN", "\\)");
    lex.skip("\\s+");

    TokenStream ts(lex, "(3 + 4) * 2");

    ts.expect("LPAREN");
    auto lhs = ts.expect("NUM");
    ts.expect("OP");
    auto rhs = ts.expect("NUM");
    ts.expect("RPAREN");
    ts.expect("OP");
    ts.expect("NUM");
    ts.expect_eof();

    ASSERT_EQ(lhs.value, "3");
    ASSERT_EQ(rhs.value, "4");
}

TEST(integration_checkpoint_backtrack)
{
    Lexer lex;
    lex.add_rule("KEYWORD", "let");
    lex.add_rule("IDENT", "[a-zA-Z_][a-zA-Z0-9_]*");
    lex.add_rule("EQ", "=");
    lex.add_rule("NUM", "[0-9]+");
    lex.skip("\\s+");

    {
        TokenStream ts(lex, "let x = 42");
        ts.save();
        bool has_let = ts.try_consume("KEYWORD").has_value();
        if (!has_let)
            ts.restore();
        else
            ts.commit();

        auto name = ts.expect("IDENT");
        ts.expect("EQ");
        auto val = ts.expect("NUM");

        ASSERT_TRUE(has_let);
        ASSERT_EQ(name.value, "x");
        ASSERT_EQ(val.value, "42");
    }

    {
        TokenStream ts(lex, "y = 7");
        ts.save();
        bool has_let = ts.try_consume("KEYWORD").has_value();
        if (!has_let)
            ts.restore();
        else
            ts.commit();

        auto name = ts.expect("IDENT");
        ts.expect("EQ");
        auto val = ts.expect("NUM");

        ASSERT_FALSE(has_let);
        ASSERT_EQ(name.value, "y");
        ASSERT_EQ(val.value, "7");
    }
}

TEST(integration_split_join_roundtrip)
{
    std::string original = "one::two::three::four";
    auto segs = splitter::split_with_delimiters(original, "::");
    std::string rebuilt = splitter::Splitter::join(segs);
    ASSERT_EQ(rebuilt, original);
}

TEST(integration_analyze_then_cache)
{
    std::string pattern = "(a+b+)+";
    auto info = analyzer::analyse(pattern);
    ASSERT_TRUE(info.potentially_catastrophic);
    ASSERT_GE(info.star_height, 2);

    auto& gc = cache::global_cache();
    size_t before = gc.size();

    nfa::NFA n;
    bool hit = gc.get_nfa(pattern, Flag::NONE, n);
    if (!hit) {
        Regex r(pattern);
        gc.put_nfa(pattern, Flag::NONE, r.nfa_ref());
    }

    nfa::NFA n2;
    bool hit2 = gc.get_nfa(pattern, Flag::NONE, n2);
    ASSERT_TRUE(hit2);
    (void)before;
}

TEST(integration_find_all_then_rewrite)
{
    Regex r("[0-9]+");
    std::string text = "order 3 units and 12 extras";
    auto matches = r.find_all_matches(text);
    ASSERT_EQ(matches.size(), 2u);

    std::string result = rewriter::replace_all(r, text, "(\\0)");
    ASSERT_NE(result.find("(3)"), std::string::npos);
    ASSERT_NE(result.find("(12)"), std::string::npos);
}

TEST(integration_collect_bracketed_tokens)
{
    Lexer lex;
    lex.add_rule("IDENT", "[a-z]+");
    lex.add_rule("LPAREN", "\\(");
    lex.add_rule("RPAREN", "\\)");
    lex.add_rule("COMMA", ",");
    lex.skip("\\s+");

    TokenStream ts(lex, "foo(a, b, c)");
    ts.expect("IDENT");
    ts.expect("LPAREN");
    auto args = ts.collect_until("RPAREN");
    ts.expect("RPAREN");
    ts.expect_eof();

    size_t ident_count = 0;
    for (auto& t : args)
        if (t.type == "IDENT") ++ident_count;
    ASSERT_EQ(ident_count, 3u);
}

TEST(integration_split_on_alternation)
{
    auto parts = splitter::split("a,b;c,d", "[,;]");
    ASSERT_EQ(parts.size(), 4u);
    ASSERT_EQ(parts[0], "a");
    ASSERT_EQ(parts[1], "b");
    ASSERT_EQ(parts[2], "c");
    ASSERT_EQ(parts[3], "d");
}

TEST(integration_replace_fn_multiply_numbers)
{
    Regex r("[0-9]+");
    std::string result =
        rewriter::replace_fn(r, "a2b3c4", [](const nfa::MatchData& md, const std::string& src) -> std::string {
            int v = std::stoi(src.substr(md.start, md.end - md.start));
            return std::to_string(v * 10);
        });
    ASSERT_NE(result.find("20"), std::string::npos);
    ASSERT_NE(result.find("30"), std::string::npos);
    ASSERT_NE(result.find("40"), std::string::npos);
}

TEST(integration_skip_then_collect)
{
    Lexer lex;
    lex.add_rule("SPACE", "\\s+");
    lex.add_rule("WORD", "[a-z]+");
    lex.add_rule("NUM", "[0-9]+");

    TokenStream ts(lex, "  hello 42 world");

    ts.skip_while([](const LexToken& t) { return t.type == "SPACE"; });

    auto words = ts.collect_while([](const LexToken& t) { return t.type == "WORD"; });
    ASSERT_EQ(words.size(), 1u);
    ASSERT_EQ(words[0].value, "hello");
}
