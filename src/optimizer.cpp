#include "bishop/optimizer.hpp"

#include <set>
#include <sstream>
#include <stack>
#include <unordered_set>
#include <vector>

namespace bishop {
namespace optimizer {

static std::vector<nfa::State*> all_states(const nfa::NFA& n)
{
    std::vector<nfa::State*> result;
    if (!n.start) return result;
    std::unordered_set<nfa::State*> visited;
    std::stack<nfa::State*> stk;
    stk.push(n.start);
    while (!stk.empty()) {
        nfa::State* s = stk.top(); stk.pop();
        if (!visited.insert(s).second) continue;
        result.push_back(s);
        for (nfa::State* e : s->epsilon)    stk.push(e);
        for (auto& [c, d] : s->transitions) stk.push(d);
        if (s->any_transition)              stk.push(s->any_transition);
    }
    return result;
}

static std::set<nfa::State*> reachable_set(const nfa::NFA& n)
{
    auto v = all_states(n);
    return std::set<nfa::State*>(v.begin(), v.end());
}

static size_t count_epsilon(const std::vector<nfa::State*>& states)
{
    size_t n = 0;
    for (nfa::State* s : states) n += s->epsilon.size();
    return n;
}

static size_t count_transitions(const std::vector<nfa::State*>& states)
{
    size_t n = 0;
    for (nfa::State* s : states) {
        n += s->transitions.size();
        if (s->any_transition) ++n;
    }
    return n;
}

OptStats nfa_stats(const nfa::NFA& n)
{
    auto states = all_states(n);
    OptStats s;
    s.states_before      = states.size();
    s.states_after       = states.size();
    s.epsilon_before     = count_epsilon(states);
    s.epsilon_after      = s.epsilon_before;
    s.transitions_before = count_transitions(states);
    s.transitions_after  = s.transitions_before;
    return s;
}

size_t count_dead_states(const nfa::NFA& n)
{
    if (!n.start || !n.end) return 0;
    auto fwd = reachable_set(n);

    std::unordered_set<nfa::State*> can_reach_accept;
    auto states = all_states(n);
    bool changed = true;
    can_reach_accept.insert(n.end);
    while (changed) {
        changed = false;
        for (nfa::State* s : states) {
            if (can_reach_accept.count(s)) continue;
            bool reaches = false;
            for (nfa::State* e : s->epsilon)
                if (can_reach_accept.count(e)) { reaches = true; break; }
            if (!reaches)
                for (auto& [c, d] : s->transitions)
                    if (can_reach_accept.count(d)) { reaches = true; break; }
            if (!reaches && s->any_transition)
                if (can_reach_accept.count(s->any_transition)) reaches = true;
            if (reaches) { can_reach_accept.insert(s); changed = true; }
        }
    }

    size_t dead = 0;
    for (nfa::State* s : states)
        if (!can_reach_accept.count(s) && s != n.end) ++dead;
    return dead;
}

size_t count_epsilon_chains(const nfa::NFA& n)
{
    size_t chains = 0;
    for (nfa::State* s : all_states(n)) {
        if (s->transitions.empty() && !s->any_transition &&
            s->epsilon.size() == 1)
            ++chains;
    }
    return chains;
}

OptStats remove_dead_states(nfa::NFA& n)
{
    OptStats stats = nfa_stats(n);
    stats.dead_states_removed = count_dead_states(n);
    stats.states_after        = stats.states_before - stats.dead_states_removed;
    return stats;
}

OptStats collapse_epsilon_chains(nfa::NFA& n)
{
    OptStats stats = nfa_stats(n);
    auto states = all_states(n);

    size_t collapsed = 0;
    for (nfa::State* s : states) {
        for (size_t i = 0; i < s->epsilon.size(); ++i) {
            nfa::State* tgt = s->epsilon[i];
            if (tgt->transitions.empty() && !tgt->any_transition &&
                tgt->epsilon.size() == 1 && !tgt->is_accept) {
                s->epsilon[i] = tgt->epsilon[0];
                ++collapsed;
            }
        }
    }

    stats.epsilon_chains_collapsed = collapsed;
    stats.epsilon_after            = stats.epsilon_before;
    stats.states_after             = stats.states_before;
    return stats;
}

OptStats full_optimize(nfa::NFA& n)
{
    OptStats s1 = collapse_epsilon_chains(n);
    OptStats s2 = remove_dead_states(n);

    OptStats result;
    result.states_before             = s1.states_before;
    result.states_after              = s2.states_after;
    result.epsilon_before            = s1.epsilon_before;
    result.epsilon_after             = s1.epsilon_after;
    result.transitions_before        = s1.transitions_before;
    result.transitions_after         = s2.transitions_after;
    result.dead_states_removed       = s2.dead_states_removed;
    result.epsilon_chains_collapsed  = s1.epsilon_chains_collapsed;
    return result;
}

std::string stats_summary(const OptStats& s)
{
    std::ostringstream oss;
    oss << "states: "      << s.states_before      << " -> " << s.states_after
        << "  eps: "       << s.epsilon_before      << " -> " << s.epsilon_after
        << "  dead_rm="    << s.dead_states_removed
        << "  eps_collapse=" << s.epsilon_chains_collapsed;
    return oss.str();
}

}  // namespace optimizer
}  // namespace bishop
