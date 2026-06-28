#include "bishop/analyzer.hpp"
#include "bishop/tokenizer.hpp"

#include <algorithm>
#include <sstream>

namespace bishop {
namespace analyzer {

using tokenizer::TokenType;

int star_height(const std::vector<tokenizer::Token>& tokens)
{
    int depth = 0, max_depth = 0;
    for (const auto& t : tokens) {
        if (t.type == TokenType::LParen || t.type == TokenType::GroupStart)
            ++depth;
        else if (t.type == TokenType::RParen)
            --depth;
        else if (t.type == TokenType::Star ||
                 t.type == TokenType::Plus ||
                 t.type == TokenType::Question ||
                 t.type == TokenType::Repeat)
            max_depth = std::max(max_depth, depth + 1);
    }
    return max_depth;
}

int nesting_depth(const std::vector<tokenizer::Token>& tokens)
{
    int depth = 0, max_depth = 0;
    for (const auto& t : tokens) {
        if (t.type == TokenType::LParen || t.type == TokenType::GroupStart ||
            t.type == TokenType::NamedGroup)
            { ++depth; max_depth = std::max(max_depth, depth); }
        else if (t.type == TokenType::RParen)
            --depth;
    }
    return max_depth;
}

int group_count(const std::vector<tokenizer::Token>& tokens)
{
    int count = 0;
    for (const auto& t : tokens)
        if (t.type == TokenType::GroupStart ||
            t.type == TokenType::NamedGroup)
            ++count;
    return count;
}

size_t estimate_state_count(const std::vector<tokenizer::Token>& postfix)
{
    size_t states = 0;
    for (const auto& t : postfix) {
        switch (t.type) {
            case TokenType::Literal:
            case TokenType::Dot:
            case TokenType::CharClass:   states += 2;  break;
            case TokenType::Star:
            case TokenType::Plus:
            case TokenType::Question:    states += 2;  break;
            case TokenType::Union:       states += 2;  break;
            case TokenType::Repeat:
                states += 2 * static_cast<size_t>(
                              std::max(1, t.repeat_max < 0
                                       ? t.repeat_min + 2
                                       : t.repeat_max));
                break;
            default: break;
        }
    }
    return states;
}

bool has_catastrophic_backtracking(const std::string& pattern)
{
    auto tokens = tokenizer::tokenize(pattern);
    int star_h  = star_height(tokens);
    if (star_h < 2) return false;

    int nesting = nesting_depth(tokens);
    bool has_union = false;
    for (const auto& t : tokens)
        if (t.type == TokenType::Union) { has_union = true; break; }

    return star_h >= 2 && nesting >= 2 && has_union;
}

int complexity_score(const PatternInfo& info)
{
    int score = 0;
    score += static_cast<int>(info.estimated_states) / 2;
    score += info.star_height * 10;
    score += info.nesting_depth * 5;
    score += info.alternation_count * 3;
    score += info.quantifier_count * 2;
    score += info.group_count * 2;
    if (info.uses_dot)         score += 3;
    if (info.uses_lookahead)   score += 8;
    if (info.potentially_catastrophic) score += 30;
    return std::min(score, 100);
}

PatternInfo analyse(const std::string& pattern)
{
    PatternInfo info;
    auto tokens = tokenizer::tokenize(pattern);

    tokenizer::AnchorInfo ai = tokenizer::strip_anchors(tokens);
    info.uses_anchor_start = ai.start;
    info.uses_anchor_end   = ai.end;

    auto with_concats = tokenizer::insert_concats(tokens);
    auto postfix      = tokenizer::to_postfix(with_concats);

    info.estimated_states  = estimate_state_count(postfix);
    info.star_height       = star_height(tokens);
    info.nesting_depth     = nesting_depth(tokens);
    info.group_count       = group_count(tokens);

    for (const auto& t : tokens) {
        switch (t.type) {
            case TokenType::Union:       ++info.alternation_count; break;
            case TokenType::Star:
            case TokenType::Plus:
            case TokenType::Question:
            case TokenType::Repeat:      ++info.quantifier_count;  break;
            case TokenType::Dot:         info.uses_dot        = true; break;
            case TokenType::CharClass:   info.uses_charclass  = true; break;
            case TokenType::NamedGroup:  info.uses_named_groups = true; break;
            case TokenType::Lookahead:
            case TokenType::NegLookahead: info.uses_lookahead = true; break;
            default: break;
        }
    }

    info.potentially_catastrophic = has_catastrophic_backtracking(pattern);
    info.complexity_score         = complexity_score(info);
    return info;
}

std::string pattern_summary(const PatternInfo& info)
{
    std::ostringstream oss;
    oss << "states~"      << info.estimated_states
        << " depth="      << info.nesting_depth
        << " star_h="     << info.star_height
        << " groups="     << info.group_count
        << " score="      << info.complexity_score;
    if (info.potentially_catastrophic) oss << " [CATASTROPHIC]";
    if (info.uses_anchor_start)        oss << " ^";
    if (info.uses_anchor_end)          oss << " $";
    return oss.str();
}

}  // namespace analyzer
}  // namespace bishop
