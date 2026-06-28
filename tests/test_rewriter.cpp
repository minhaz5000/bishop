#include "bishop/regex.hpp"
#include "bishop/rewriter.hpp"
#include "framework.hpp"

using namespace bishop;
using namespace bishop::rewriter;

TEST(parse_empty_replacement)
{
    auto parts = parse_replacement("");
    ASSERT_TRUE(parts.empty());
}

TEST(parse_literal_only)
{
    auto parts = parse_replacement("hello world");
    ASSERT_EQ(parts.size(), 1u);
    ASSERT_EQ(parts[0].kind, ReplacePart::Kind::Literal);
    ASSERT_EQ(parts[0].text, "hello world");
}

TEST(parse_backslash_zero)
{
    auto parts = parse_replacement("\\0");
    ASSERT_EQ(parts.size(), 1u);
    ASSERT_EQ(parts[0].kind, ReplacePart::Kind::WholeMatch);
}

TEST(parse_backslash_ampersand)
{
    auto parts = parse_replacement("\\&");
    ASSERT_EQ(parts.size(), 1u);
    ASSERT_EQ(parts[0].kind, ReplacePart::Kind::WholeMatch);
}

TEST(parse_numbered_groups)
{
    auto parts = parse_replacement("\\1-\\2-\\3");
    ASSERT_EQ(parts.size(), 5u);
    ASSERT_EQ(parts[0].kind, ReplacePart::Kind::GroupIndex);
    ASSERT_EQ(parts[0].index, 1);
    ASSERT_EQ(parts[1].kind, ReplacePart::Kind::Literal);
    ASSERT_EQ(parts[1].text, "-");
    ASSERT_EQ(parts[2].kind, ReplacePart::Kind::GroupIndex);
    ASSERT_EQ(parts[2].index, 2);
    ASSERT_EQ(parts[4].kind, ReplacePart::Kind::GroupIndex);
    ASSERT_EQ(parts[4].index, 3);
}

TEST(parse_dollar_brace_zero)
{
    auto parts = parse_replacement("${0}");
    ASSERT_EQ(parts.size(), 1u);
    ASSERT_EQ(parts[0].kind, ReplacePart::Kind::WholeMatch);
}

TEST(parse_dollar_brace_numbered)
{
    auto parts = parse_replacement("${1}_${2}");
    ASSERT_EQ(parts.size(), 3u);
    ASSERT_EQ(parts[0].kind, ReplacePart::Kind::GroupIndex);
    ASSERT_EQ(parts[0].index, 1);
    ASSERT_EQ(parts[1].kind, ReplacePart::Kind::Literal);
    ASSERT_EQ(parts[1].text, "_");
    ASSERT_EQ(parts[2].kind, ReplacePart::Kind::GroupIndex);
    ASSERT_EQ(parts[2].index, 2);
}

TEST(parse_dollar_brace_named)
{
    auto parts = parse_replacement("${year}-${month}");
    ASSERT_EQ(parts.size(), 3u);
    ASSERT_EQ(parts[0].kind, ReplacePart::Kind::GroupName);
    ASSERT_EQ(parts[0].name, "year");
    ASSERT_EQ(parts[2].kind, ReplacePart::Kind::GroupName);
    ASSERT_EQ(parts[2].name, "month");
}

TEST(parse_escaped_backslash)
{
    auto parts = parse_replacement("\\\\");
    ASSERT_EQ(parts.size(), 1u);
    ASSERT_EQ(parts[0].kind, ReplacePart::Kind::Literal);
    ASSERT_EQ(parts[0].text, "\\");
}

TEST(parse_mixed_template)
{
    auto parts = parse_replacement("[\\1] matched at \\0");
    bool found_group = false, found_whole = false;
    for (auto& p : parts) {
        if (p.kind == ReplacePart::Kind::GroupIndex && p.index == 1) found_group = true;
        if (p.kind == ReplacePart::Kind::WholeMatch) found_whole = true;
    }
    ASSERT_TRUE(found_group);
    ASSERT_TRUE(found_whole);
}

TEST(parse_unclosed_dollar_brace_is_literal)
{
    auto parts = parse_replacement("${name");
    bool has_literal_dollar = false;
    for (auto& p : parts)
        if (p.kind == ReplacePart::Kind::Literal && p.text.find('$') != std::string::npos) has_literal_dollar = true;
    ASSERT_TRUE(has_literal_dollar);
}

TEST(replace_first_no_match_returns_original)
{
    Regex r("xyz");
    std::string result = replace_first(r, "hello", "REPLACED");
    ASSERT_EQ(result, "hello");
}

TEST(replace_first_literal_replacement)
{
    Regex r("world");
    std::string result = replace_first(r, "hello world", "there");
    ASSERT_EQ(result, "hello there");
}

TEST(replace_first_whole_match_backref)
{
    Regex r("[a-z]+");
    std::string result = replace_first(r, "hello world", "[\\0]");
    ASSERT_EQ(result, "[hello] world");
}

TEST(replace_first_whole_match_dollar)
{
    Regex r("[0-9]+");
    std::string result = replace_first(r, "abc 42 def", "<${0}>");
    ASSERT_EQ(result, "abc <42> def");
}

TEST(replace_first_only_replaces_first)
{
    Regex r("[0-9]+");
    std::string result = replace_first(r, "1 2 3", "X");
    ASSERT_EQ(result, "X 2 3");
}

TEST(replace_first_at_start)
{
    Regex r("[a-z]+");
    std::string result = replace_first(r, "foo bar", "BAZ");
    ASSERT_EQ(result, "BAZ bar");
}

TEST(replace_first_at_end)
{
    Regex r("[0-9]+$");
    std::string result = replace_first(r, "abc123", "NUM");
    ASSERT_EQ(result, "abcNUM");
}

TEST(replace_all_no_match_returns_original)
{
    Regex r("xyz");
    std::string result = replace_all(r, "hello world", "X");
    ASSERT_EQ(result, "hello world");
}

TEST(replace_all_literal_replacement)
{
    Regex r("[aeiou]");
    std::string result = replace_all(r, "hello", "*");
    ASSERT_EQ(result, "h*ll*");
}

TEST(replace_all_whole_match_wraps_each)
{
    Regex r("[0-9]+");
    std::string result = replace_all(r, "1 22 333", "<\\0>");
    ASSERT_EQ(result, "<1> <22> <333>");
}

TEST(replace_all_dollar_whole_match)
{
    Regex r("\\w+");
    std::string result = replace_all(r, "foo bar", "[${0}]");
    ASSERT_EQ(result, "[foo] [bar]");
}

TEST(replace_fn_can_uppercase_matches)
{
    Regex r("[a-z]+");
    auto result = replace_fn(r, "hello world", [](const nfa::MatchData& md, const std::string& src) -> std::string {
        std::string s = src.substr(md.start, md.end - md.start);
        for (char& c : s) c = static_cast<char>(std::toupper((unsigned char)c));
        return s;
    });
    ASSERT_EQ(result, "HELLO WORLD");
}

TEST(replace_fn_count_matches)
{
    Regex r("[0-9]");
    int count = 0;
    replace_fn(r, "a1b2c3", [&](const nfa::MatchData&, const std::string&) -> std::string {
        ++count;
        return "X";
    });
    ASSERT_EQ(count, 3);
}

TEST(replace_fn_receives_correct_positions)
{
    Regex r("[a-z]+");
    std::vector<std::pair<size_t, size_t>> positions;
    replace_fn(r, "abc 123 def", [&](const nfa::MatchData& md, const std::string&) -> std::string {
        positions.push_back({md.start, md.end});
        return "";
    });
    ASSERT_EQ(positions.size(), 2u);
    ASSERT_EQ(positions[0].first, 0u);
    ASSERT_EQ(positions[0].second, 3u);
    ASSERT_EQ(positions[1].first, 8u);
    ASSERT_EQ(positions[1].second, 11u);
}

TEST(dump_replacement_literal)
{
    auto parts = parse_replacement("hello");
    std::string d = dump_replacement(parts);
    ASSERT_FALSE(d.empty());
    ASSERT_NE(d.find("Literal"), std::string::npos);
}

TEST(dump_replacement_mixed)
{
    auto parts = parse_replacement("\\1-\\0");
    std::string d = dump_replacement(parts);
    ASSERT_NE(d.find("Group"), std::string::npos);
    ASSERT_NE(d.find("WholeMatch"), std::string::npos);
}

TEST(nfa_match_captures_no_groups)
{
    Regex r("hello");
    auto md = r.search_match("say hello world");
    ASSERT_TRUE(md.matched);
    ASSERT_EQ(md.start, 4u);
    ASSERT_EQ(md.end, 9u);
    ASSERT_TRUE(md.groups.empty());
}

TEST(nfa_match_captures_no_match)
{
    Regex r("xyz");
    auto md = r.search_match("hello world");
    ASSERT_FALSE(md.matched);
}

TEST(find_all_matches_returns_all)
{
    Regex r("[0-9]+");
    auto all = r.find_all_matches("1 22 333");
    ASSERT_EQ(all.size(), 3u);
    ASSERT_EQ(all[0].start, 0u);
    ASSERT_EQ(all[0].end, 1u);
    ASSERT_EQ(all[1].start, 2u);
    ASSERT_EQ(all[1].end, 4u);
    ASSERT_EQ(all[2].start, 5u);
    ASSERT_EQ(all[2].end, 8u);
}

TEST(find_all_matches_empty_on_no_match)
{
    Regex r("xyz");
    auto all = r.find_all_matches("hello");
    ASSERT_TRUE(all.empty());
}
