#ifndef NFA_H_
#define NFA_H_

#include <set>
#include <vector>

#include "tokenizer.hpp"

namespace nfa {

struct State {
    int id;
    bool is_accept;
    std::unordered_map<char, State*> transitions;
    std::vector<State*> epsilon;
};

struct NFA {
    State* start;
    State* end;
};

NFA create_nfa_concat(int& id, NFA first, NFA second);
NFA create_nfa_literal(int& id, char ch);
NFA create_nfa_question(int& id, NFA first);
NFA create_nfa_plus(int& id, NFA first);
NFA create_nfa_kleene_star(int& id, NFA first);
NFA create_nfa_union(int& id, NFA first, NFA second);
NFA parse_postfix(const std::vector<tokenizer::Token>& postfix);

std::set<State*> epsilon_closure(const std::set<State*>& states);

bool nfa_match(NFA& nfa, const std::string& input);

}  // namespace nfa
#endif
