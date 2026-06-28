#ifndef BISHOP_OPTIMIZER_HPP_
#define BISHOP_OPTIMIZER_HPP_

#include <cstddef>
#include <string>

#include "bishop/nfa.hpp"

namespace bishop {
namespace optimizer {

struct OptStats {
    size_t states_before{0};
    size_t states_after{0};
    size_t transitions_before{0};
    size_t transitions_after{0};
    size_t epsilon_before{0};
    size_t epsilon_after{0};
    size_t dead_states_removed{0};
    size_t epsilon_chains_collapsed{0};
};

OptStats   nfa_stats(const nfa::NFA& n);
size_t     count_dead_states(const nfa::NFA& n);
size_t     count_epsilon_chains(const nfa::NFA& n);
OptStats   remove_dead_states(nfa::NFA& n);
OptStats   collapse_epsilon_chains(nfa::NFA& n);
OptStats   full_optimize(nfa::NFA& n);
std::string stats_summary(const OptStats& s);

}  // namespace optimizer
}  // namespace bishop
#endif
