#include "bishop/dfa.hpp"

#include <map>
#include <queue>
#include <set>

namespace dfa {

using StateSet = std::set<nfa::State*>;

static StateSet move_on(const StateSet& ss, unsigned char c) {
    StateSet result;
    for (nfa::State* s : ss) {
        auto it = s->transitions.find(static_cast<char>(c));
        if (it != s->transitions.end())
            result.insert(it->second);
        if (s->any_transition)
            result.insert(s->any_transition);
    }
    return result;
}

static bool any_accept(const StateSet& ss) {
    for (nfa::State* s : ss)
        if (s->is_accept) return true;
    return false;
}

DFA nfa_to_dfa(nfa::NFA& n) {
    DFA d;
    if (!n.start) return d;

    std::map<StateSet, int> seen;
    std::queue<int>         worklist;

    StateSet s0 = nfa::epsilon_closure({n.start});
    seen[s0]    = 0;
    d.states.push_back({0, any_accept(s0), {}});
    d.start = 0;
    d.valid = true;
    worklist.push(0);

    std::vector<StateSet> id_to_set;
    id_to_set.push_back(s0);

    while (!worklist.empty()) {
        int sid = worklist.front(); worklist.pop();
        const StateSet& cur = id_to_set[sid];

        for (int c = 0; c < 256; ++c) {
            StateSet mv = move_on(cur, static_cast<unsigned char>(c));
            if (mv.empty()) continue;
            StateSet nx = nfa::epsilon_closure(mv);
            if (nx.empty()) continue;

            auto it = seen.find(nx);
            int  nid;
            if (it == seen.end()) {
                nid      = static_cast<int>(d.states.size());
                seen[nx] = nid;
                id_to_set.push_back(nx);
                d.states.push_back({nid, any_accept(nx), {}});
                worklist.push(nid);
            } else {
                nid = it->second;
            }
            d.states[sid].transitions[static_cast<unsigned char>(c)] = nid;
        }
    }
    return d;
}

bool dfa_match(const DFA& d, const std::string& input) {
    if (!d.valid || d.states.empty()) return false;
    int cur = d.start;
    for (unsigned char c : input) {
        auto it = d.states[cur].transitions.find(c);
        if (it == d.states[cur].transitions.end()) return false;
        cur = it->second;
    }
    return d.states[cur].is_accept;
}

DFA minimize(const DFA& d) {
    int n = static_cast<int>(d.states.size());
    if (n == 0) return d;

    std::vector<int> cls(n);
    for (int i = 0; i < n; ++i)
        cls[i] = d.states[i].is_accept ? 1 : 0;

    bool changed = true;
    while (changed) {
        changed = false;
        std::map<std::vector<int>, int> sig_map;
        std::vector<int> next_cls(n);
        int counter = 0;

        for (int i = 0; i < n; ++i) {
            std::vector<int> sig;
            sig.reserve(257);
            sig.push_back(cls[i]);
            for (int c = 0; c < 256; ++c) {
                auto it = d.states[i].transitions.find(static_cast<unsigned char>(c));
                sig.push_back(it != d.states[i].transitions.end()
                              ? cls[it->second] : -1);
            }
            auto jt = sig_map.find(sig);
            if (jt == sig_map.end())
                sig_map[sig] = next_cls[i] = counter++;
            else
                next_cls[i] = jt->second;
        }

        if (next_cls != cls) { cls = std::move(next_cls); changed = true; }
    }

    int num = *std::max_element(cls.begin(), cls.end()) + 1;
    DFA result;
    result.states.resize(num);
    result.start = cls[d.start];
    result.valid = d.valid;

    for (int i = 0; i < n; ++i) {
        int c = cls[i];
        result.states[c].id        = c;
        result.states[c].is_accept = d.states[i].is_accept;
        for (auto& [ch, dst] : d.states[i].transitions)
            result.states[c].transitions[ch] = cls[dst];
    }
    return result;
}

}  // namespace dfa
