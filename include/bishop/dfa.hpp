#ifndef BISHOP_DFA_HPP_
#define BISHOP_DFA_HPP_

#include <string>
#include <unordered_map>
#include <vector>

#include "bishop/nfa.hpp"

namespace dfa {

struct DFAState {
    int  id;
    bool is_accept;
    std::unordered_map<unsigned char, int> transitions;
};

struct DFA {
    std::vector<DFAState> states;
    int  start{0};
    bool valid{false};
};

DFA  nfa_to_dfa(nfa::NFA& n);
DFA  minimize(const DFA& d);
bool dfa_match(const DFA& d, const std::string& input);

}  // namespace dfa
#endif
