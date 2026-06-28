#ifndef BISHOP_ANALYZER_HPP_
#define BISHOP_ANALYZER_HPP_

#include <string>
#include <vector>

#include "bishop/nfa.hpp"
#include "bishop/tokenizer.hpp"

namespace bishop {
namespace analyzer {

struct PatternInfo {
    size_t estimated_states{0};
    int star_height{0};
    int nesting_depth{0};
    int group_count{0};
    int alternation_count{0};
    int quantifier_count{0};
    bool uses_dot{false};
    bool uses_anchor_start{false};
    bool uses_anchor_end{false};
    bool uses_charclass{false};
    bool uses_named_groups{false};
    bool uses_lookahead{false};
    bool uses_backreference{false};
    int complexity_score{0};
    bool potentially_catastrophic{false};
};

PatternInfo analyse(const std::string& pattern);
int star_height(const std::vector<tokenizer::Token>& tokens);
int nesting_depth(const std::vector<tokenizer::Token>& tokens);
int group_count(const std::vector<tokenizer::Token>& tokens);
bool has_catastrophic_backtracking(const std::string& pattern);
int complexity_score(const PatternInfo& info);
size_t estimate_state_count(const std::vector<tokenizer::Token>& postfix);
std::string pattern_summary(const PatternInfo& info);

}  // namespace analyzer
}  // namespace bishop
#endif
