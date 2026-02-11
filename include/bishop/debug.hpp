#ifndef BISHOP_DEBUG_HPP_
#define BISHOP_DEBUG_HPP_

#include <ostream>
#include <set>
#include <string>
#include <unordered_set>

#include "bishop/dfa.hpp"
#include "bishop/nfa.hpp"
#include "bishop/utils.hpp"

namespace bishop {
namespace debug {

void print_nfa_dot(const nfa::NFA& n, std::ostream& out,
                   const std::string& graph_name = "NFA");

void print_dfa_dot(const dfa::DFA& d, std::ostream& out,
                   const std::string& graph_name = "DFA");

void print_nfa_text(const nfa::NFA& n, std::ostream& out);
void print_dfa_text(const dfa::DFA& d, std::ostream& out);

std::string nfa_to_dot(const nfa::NFA& n,
                       const std::string& graph_name = "NFA");
std::string dfa_to_dot(const dfa::DFA& d,
                       const std::string& graph_name = "DFA");

size_t nfa_state_count(const nfa::NFA& n);
size_t nfa_transition_count(const nfa::NFA& n);

}  // namespace debug
}  // namespace bishop
#endif
