#include "bishop/lexer.hpp"
#include "bishop/token_stream.hpp"
#include "framework.hpp"

using namespace bishop;

static Lexer make_arith_lexer()
{
    Lexer lex;
    lex.add_rule("NUM", "[0-9]+");
    lex.add_rule("IDENT", "[a-zA-Z_][a-zA-Z0-9_]*");
    lex.add_rule("PLUS", "\\+");
    lex.add_rule("MINUS", "-");
    lex.add_rule("STAR", "\\*");
    lex.add_rule("SLASH", "/");
    lex.add_rule("LPAREN", "\\(");
    lex.add_rule("RPAREN", "\\)");
    lex.add_rule("EQ", "=");
    lex.add_rule("COMMA", ",");
    lex.skip("\\s+");
    return lex;
}

TEST(construct_from_tokens)
{
    std::vector<LexToken> toks;
    LexToken t;
    t.type = "NUM";
    t.value = "42";
    toks.push_back(t);
    TokenStream ts(std::move(toks));
    ASSERT_FALSE(ts.eof());
    ASSERT_EQ(ts.size(), 1u);
}

TEST(construct_from_lexer)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ASSERT_FALSE(ts.eof());
    ASSERT_EQ(ts.size(), 3u);
}

TEST(empty_stream_is_eof)
{
    TokenStream ts;
    ASSERT_TRUE(ts.eof());
    ASSERT_EQ(ts.size(), 0u);
    ASSERT_EQ(ts.remaining(), 0u);
}

TEST(pos_starts_at_zero)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ASSERT_EQ(ts.pos(), 0u);
}

TEST(remaining_decreases_after_consume)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ASSERT_EQ(ts.remaining(), 3u);
    ts.consume();
    ASSERT_EQ(ts.remaining(), 2u);
    ts.consume();
    ASSERT_EQ(ts.remaining(), 1u);
    ts.consume();
    ASSERT_EQ(ts.remaining(), 0u);
    ASSERT_TRUE(ts.eof());
}

TEST(peek_returns_current_without_advancing)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "42");
    const LexToken& t = ts.peek();
    ASSERT_EQ(t.type, "NUM");
    ASSERT_EQ(t.value, "42");
    ASSERT_EQ(ts.pos(), 0u);
}

TEST(peek_at_offset)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ASSERT_EQ(ts.peek_at(0)->type, "NUM");
    ASSERT_EQ(ts.peek_at(1)->type, "PLUS");
    ASSERT_EQ(ts.peek_at(2)->type, "NUM");
    ASSERT_EQ(ts.peek_at(3), nullptr);
}

TEST(peek_is_correct)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ASSERT_TRUE(ts.peek_is("NUM"));
    ASSERT_FALSE(ts.peek_is("PLUS"));
}

TEST(peek_on_eof_throws)
{
    TokenStream ts;
    ASSERT_THROWS(ts.peek());
}

TEST(consume_advances_position)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    auto t = ts.consume();
    ASSERT_EQ(t.type, "NUM");
    ASSERT_EQ(ts.pos(), 1u);
}

TEST(consume_on_eof_throws)
{
    TokenStream ts;
    ASSERT_THROWS(ts.consume());
}

TEST(expect_correct_type)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "42 + 7");
    auto num = ts.expect("NUM");
    ASSERT_EQ(num.value, "42");
    ts.expect("PLUS");
    auto rhs = ts.expect("NUM");
    ASSERT_EQ(rhs.value, "7");
    ts.expect_eof();
}

TEST(expect_wrong_type_throws)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "42");
    ASSERT_THROWS(ts.expect("PLUS"));
}

TEST(expect_eof_when_empty)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1");
    ts.consume();
    ASSERT_NO_THROW(ts.expect_eof());
}

TEST(expect_eof_when_not_empty_throws)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ts.consume();
    ASSERT_THROWS(ts.expect_eof());
}

TEST(try_consume_success)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    auto result = ts.try_consume("NUM");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->value, "1");
    ASSERT_EQ(ts.pos(), 1u);
}

TEST(try_consume_failure_no_advance)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    auto result = ts.try_consume("PLUS");
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(ts.pos(), 0u);
}

TEST(try_consume_on_eof)
{
    TokenStream ts;
    auto result = ts.try_consume("NUM");
    ASSERT_FALSE(result.has_value());
}

TEST(skip_advances_by_n)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ts.skip(2);
    ASSERT_EQ(ts.pos(), 2u);
    ASSERT_EQ(ts.peek().type, "NUM");
    ASSERT_EQ(ts.peek().value, "2");
}

TEST(skip_past_end_clamps)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ts.skip(100);
    ASSERT_TRUE(ts.eof());
}

TEST(skip_while_stops_at_non_matching)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 2 3 + 4");
    ts.skip_while([](const LexToken& t) { return t.type == "NUM"; });
    ASSERT_EQ(ts.peek().type, "PLUS");
}

TEST(skip_all_advances_to_eof)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ts.skip_all();
    ASSERT_TRUE(ts.eof());
}

TEST(checkpoint_save_restore)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ts.save();
    ts.consume();
    ts.consume();
    ASSERT_EQ(ts.pos(), 2u);
    ts.restore();
    ASSERT_EQ(ts.pos(), 0u);
}

TEST(checkpoint_commit_disables_restore)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ts.save();
    ts.consume();
    ts.commit();
    ASSERT_THROWS(ts.restore());
}

TEST(checkpoint_nested)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    ts.save();
    ts.consume();
    ts.save();
    ts.consume();
    ts.restore();
    ASSERT_EQ(ts.pos(), 1u);
    ts.restore();
    ASSERT_EQ(ts.pos(), 0u);
}

TEST(restore_with_no_checkpoint_throws)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1");
    ASSERT_THROWS(ts.restore());
}

TEST(collect_until_stops_before_sentinel)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2 * 3");
    auto nums = ts.collect_until("STAR");
    ASSERT_EQ(nums.size(), 3u);
    ASSERT_EQ(ts.peek().type, "STAR");
}

TEST(collect_until_empty_on_immediate_sentinel)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "* 1");
    auto result = ts.collect_until("STAR");
    ASSERT_TRUE(result.empty());
    ASSERT_EQ(ts.peek().type, "STAR");
}

TEST(collect_while)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 2 3 a");
    auto nums = ts.collect_while([](const LexToken& t) { return t.type == "NUM"; });
    ASSERT_EQ(nums.size(), 3u);
    ASSERT_EQ(ts.peek().type, "IDENT");
}

TEST(slice_returns_correct_range)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2 * 3");
    auto s = ts.slice(1, 3);
    ASSERT_EQ(s.size(), 2u);
    ASSERT_EQ(s[0].type, "PLUS");
    ASSERT_EQ(s[1].type, "NUM");
}

TEST(slice_out_of_range_clamps)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1 + 2");
    auto s = ts.slice(1, 100);
    ASSERT_EQ(s.size(), 2u);
}

TEST(slice_empty_when_from_exceeds_size)
{
    Lexer lex = make_arith_lexer();
    TokenStream ts(lex, "1");
    auto s = ts.slice(10, 20);
    ASSERT_TRUE(s.empty());
}
