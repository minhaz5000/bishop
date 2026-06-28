#include "bishop/lexer.hpp"
#include "framework.hpp"

using bishop::Flag;
using bishop::Lexer;
using bishop::LexError;
using bishop::LexToken;

static Lexer make_simple_lexer()
{
    Lexer lex;
    lex.skip("[ \t\n\r]+");
    lex.add_rule("INT", "\\d+");
    lex.add_rule("IDENT", "[a-zA-Z_][a-zA-Z0-9_]*");
    lex.add_rule("PLUS", "\\+");
    lex.add_rule("MINUS", "-");
    lex.add_rule("STAR", "\\*");
    lex.add_rule("SLASH", "/");
    lex.add_rule("LPAREN", "\\(");
    lex.add_rule("RPAREN", "\\)");
    lex.add_rule("EQ", "=");
    lex.add_rule("SEMI", ";");
    return lex;
}

TEST(lexer_empty_input)
{
    Lexer lex;
    lex.add_rule("INT", "\\d+");
    auto toks = lex.scan("");
    ASSERT_TRUE(toks.empty());
}

TEST(lexer_single_int)
{
    Lexer lex;
    lex.add_rule("INT", "\\d+");
    auto toks = lex.scan("42");
    ASSERT_EQ(toks.size(), 1u);
    ASSERT_EQ(toks[0].type, "INT");
    ASSERT_EQ(toks[0].value, "42");
}

TEST(lexer_skip_whitespace)
{
    auto lex = make_simple_lexer();
    auto toks = lex.scan("  42  ");
    ASSERT_EQ(toks.size(), 1u);
    ASSERT_EQ(toks[0].value, "42");
}

TEST(lexer_multiple_tokens)
{
    auto lex = make_simple_lexer();
    auto toks = lex.scan("x = 10;");
    ASSERT_EQ(toks.size(), 4u);
    ASSERT_EQ(toks[0].type, "IDENT");
    ASSERT_EQ(toks[1].type, "EQ");
    ASSERT_EQ(toks[2].type, "INT");
    ASSERT_EQ(toks[3].type, "SEMI");
}

TEST(lexer_line_tracking)
{
    auto lex = make_simple_lexer();
    auto toks = lex.scan("a\nb");
    ASSERT_EQ(toks[0].line, 1u);
    ASSERT_EQ(toks[1].line, 2u);
}

TEST(lexer_col_tracking)
{
    auto lex = make_simple_lexer();
    auto toks = lex.scan("x = 5");
    ASSERT_EQ(toks[0].col, 1u);
    ASSERT_EQ(toks[1].col, 3u);
    ASSERT_EQ(toks[2].col, 5u);
}

TEST(lexer_offset_tracking)
{
    auto lex = make_simple_lexer();
    auto toks = lex.scan("ab cd");
    ASSERT_EQ(toks[0].offset, 0u);
    ASSERT_EQ(toks[1].offset, 3u);
}

TEST(lexer_length_field)
{
    auto lex = make_simple_lexer();
    auto toks = lex.scan("hello");
    ASSERT_EQ(toks[0].length, 5u);
}

TEST(lexer_longest_match)
{
    Lexer lex;
    lex.add_rule("IF", "if");
    lex.add_rule("IDENT", "[a-zA-Z]+");
    auto toks = lex.scan("ifx");
    ASSERT_EQ(toks.size(), 1u);
    ASSERT_EQ(toks[0].type, "IDENT");
    ASSERT_EQ(toks[0].value, "ifx");
}

TEST(lexer_keyword_before_ident)
{
    Lexer lex;
    lex.skip("[ ]+");
    lex.add_rule("IF", "if");
    lex.add_rule("IDENT", "[a-zA-Z]+");
    auto toks = lex.scan("if");
    ASSERT_EQ(toks.size(), 1u);
    ASSERT_EQ(toks[0].type, "IF");
}

TEST(lexer_rule_count)
{
    auto lex = make_simple_lexer();
    ASSERT_TRUE(lex.rule_count() > 0);
}

TEST(lexer_has_rule)
{
    auto lex = make_simple_lexer();
    ASSERT_TRUE(lex.has_rule("INT"));
    ASSERT_FALSE(lex.has_rule("NONEXISTENT"));
}

TEST(lexer_remove_rule)
{
    Lexer lex;
    lex.add_rule("A", "a");
    lex.add_rule("B", "b");
    ASSERT_TRUE(lex.has_rule("A"));
    lex.remove_rule("A");
    ASSERT_FALSE(lex.has_rule("A"));
}

TEST(lexer_clear)
{
    auto lex = make_simple_lexer();
    lex.clear();
    ASSERT_EQ(lex.rule_count(), 0u);
}

TEST(lexer_unexpected_char_throws)
{
    Lexer lex;
    lex.add_rule("INT", "\\d+");
    ASSERT_THROWS(lex.scan("abc"));
}

TEST(lexer_with_action)
{
    Lexer lex;
    lex.add_rule("INT", "\\d+", [](const std::string& s) { return "NUM:" + s; });
    auto toks = lex.scan("42");
    ASSERT_EQ(toks[0].value, "NUM:42");
}

TEST(lexer_ignorecase_flag)
{
    Lexer lex;
    lex.add_rule("KW", "hello", Flag::IGNORECASE);
    auto toks = lex.scan("HELLO");
    ASSERT_EQ(toks.size(), 1u);
    ASSERT_EQ(toks[0].type, "KW");
}

TEST(lexer_tokenize_synonym)
{
    Lexer lex;
    lex.add_rule("A", "a+");
    auto t1 = lex.scan("aaa");
    auto t2 = lex.tokenize("aaa");
    ASSERT_EQ(t1.size(), t2.size());
    ASSERT_EQ(t1[0].value, t2[0].value);
}

TEST(lexer_last_tokens_cached)
{
    Lexer lex;
    lex.add_rule("INT", "\\d+");
    lex.scan("1 2");
    ASSERT_FALSE(lex.last_tokens().empty());
}

TEST(lex_token_is_method)
{
    LexToken t;
    t.type = "INT";
    ASSERT_TRUE(t.is("INT"));
    ASSERT_FALSE(t.is("STR"));
}

TEST(lex_token_empty)
{
    LexToken t;
    ASSERT_TRUE(t.empty());
    t.type = "X";
    ASSERT_FALSE(t.empty());
}

TEST(lexer_multiline_input)
{
    auto lex = make_simple_lexer();
    auto toks = lex.scan("a\nb\nc");
    ASSERT_EQ(toks.size(), 3u);
    ASSERT_EQ(toks[2].line, 3u);
}

TEST(lexer_expr_full)
{
    auto lex = make_simple_lexer();
    auto toks = lex.scan("(x + 42) * y");
    ASSERT_EQ(toks[0].type, "LPAREN");
    ASSERT_EQ(toks[1].type, "IDENT");
    ASSERT_EQ(toks[2].type, "PLUS");
    ASSERT_EQ(toks[3].type, "INT");
    ASSERT_EQ(toks[4].type, "RPAREN");
    ASSERT_EQ(toks[5].type, "STAR");
    ASSERT_EQ(toks[6].type, "IDENT");
}

TEST(lexer_only_skip_rules_empty_output)
{
    Lexer lex;
    lex.skip("\\s+");
    auto toks = lex.scan("   \t  \n  ");
    ASSERT_TRUE(toks.empty());
}

TEST(lex_error_has_position)
{
    Lexer lex;
    lex.add_rule("INT", "\\d+");
    try {
        lex.scan("1 x");
        ASSERT_TRUE(false);
    }
    catch (const LexError& e) {
        ASSERT_TRUE(e.offset() > 0);
        ASSERT_EQ(e.line(), 1u);
    }
}
