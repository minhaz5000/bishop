#include "bishop/flags.hpp"
#ifndef NFA_H_
    #define NFA_H_

    #include <set>
    #include <unordered_map>
    #include <vector>

    #include "tokenizer.hpp"

namespace nfa {

struct NFA;

enum class BoundaryType { None, WordBoundary, NonWordBoundary };
enum class LookType { None, Ahead, NegAhead };

struct State {
    int id;
    bool is_accept;
    std::unordered_map<char, State*> transitions;
    std::vector<State*> epsilon;
    State* any_transition{nullptr};
    int group_start{-1};
    int group_end{-1};
    BoundaryType boundary{BoundaryType::None};
    LookType look{LookType::None};
    NFA* look_nfa{nullptr};
};

struct NFA {
    State* start;
    State* end;
};

NFA create_nfa_literal(int& id, char ch);
NFA create_nfa_dot(int& id);
NFA create_nfa_charclass(int& id, const std::vector<bool>& charset);
NFA create_nfa_repeat(int& id, const NFA& base, int min_rep, int max_rep);
NFA create_nfa_concat(int& id, NFA first, NFA second);
NFA create_nfa_union(int& id, NFA first, NFA second);
NFA create_nfa_kleene_star(int& id, NFA first);
NFA create_nfa_plus(int& id, NFA first);
NFA create_nfa_question(int& id, NFA first);
NFA create_nfa_group(int& id, NFA inner, int group_idx);
NFA create_nfa_lookahead(int& id, NFA inner, bool negative);
NFA create_nfa_lazy_star(int& id, NFA first);
NFA create_nfa_lazy_plus(int& id, NFA first);
NFA create_nfa_lazy_question(int& id, NFA first);
NFA parse_postfix(const std::vector<tokenizer::Token>& postfix);

std::set<State*> epsilon_closure(const std::set<State*>& states);
bool nfa_match(const NFA& nfa, const std::string& input, bishop::Flag flags = bishop::Flag::NONE);

struct SimThread {
    State* state;
    std::vector<std::pair<size_t, size_t>> captures;
};

struct MatchData {
    bool matched{false};
    size_t start{std::string::npos};
    size_t end{std::string::npos};
    std::vector<std::pair<size_t, size_t>> groups;

    size_t group_count() const noexcept
    {
        return groups.size();
    }
    bool group_matched(size_t i) const noexcept
    {
        return i < groups.size() && groups[i].first != std::string::npos && groups[i].second != std::string::npos;
    }
};

MatchData nfa_match_captures(const NFA& nfa, const std::string& input, size_t from = 0,
                             bishop::Flag flags = bishop::Flag::NONE);

}  // namespace nfa
#endif
