#include "bishop/flags.hpp"
#ifndef NFA_H_
#define NFA_H_

#include <set>
#include <vector>
#include <unordered_map>

#include "tokenizer.hpp"

namespace nfa {

enum class BoundaryType { None, WordBoundary, NonWordBoundary };

struct State {
    int id;
    bool is_accept;
    std::unordered_map<char, State*> transitions;
    std::vector<State*> epsilon;
    State* any_transition{nullptr};
    int          group_start{-1};
    int          group_end{-1};
    BoundaryType boundary{BoundaryType::None};
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
NFA create_nfa_lazy_star(int& id, NFA first);
NFA create_nfa_lazy_plus(int& id, NFA first);
NFA create_nfa_lazy_question(int& id, NFA first);
NFA parse_postfix(const std::vector<tokenizer::Token>& postfix);

std::set<State*> epsilon_closure(const std::set<State*>& states);
bool nfa_match(NFA& nfa, const std::string& input,
               bishop::Flag flags = bishop::Flag::NONE);

struct SimThread {
    State*                   state;
    std::vector<std::pair<size_t,size_t>> captures;
};

}  // namespace nfa
#endif
