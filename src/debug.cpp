#include "bishop/debug.hpp"

#include <sstream>
#include <stack>

namespace bishop {
namespace debug {

static std::vector<nfa::State*> collect_nfa_states(const nfa::NFA& n)
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
        for (nfa::State* e : s->epsilon) stk.push(e);
        for (auto& [ch, dst] : s->transitions) stk.push(dst);
        if (s->any_transition) stk.push(s->any_transition);
    }
    return result;
}

size_t nfa_state_count(const nfa::NFA& n)
{
    return collect_nfa_states(n).size();
}

size_t nfa_transition_count(const nfa::NFA& n)
{
    size_t count = 0;
    for (nfa::State* s : collect_nfa_states(n)) {
        count += s->epsilon.size();
        count += s->transitions.size();
        if (s->any_transition) ++count;
    }
    return count;
}

void print_nfa_dot(const nfa::NFA& n, std::ostream& out,
                   const std::string& graph_name)
{
    out << "digraph " << graph_name << " {\n";
    out << "  rankdir=LR;\n";
    out << "  node [shape=circle];\n";

    auto states = collect_nfa_states(n);

    for (nfa::State* s : states) {
        if (s->is_accept)
            out << "  s" << s->id << " [shape=doublecircle label=\"" << s->id << "\"];\n";
        else
            out << "  s" << s->id << " [label=\"" << s->id << "\"];\n";
    }

    out << "  start [shape=point];\n";
    if (n.start) out << "  start -> s" << n.start->id << ";\n";

    for (nfa::State* s : states) {
        for (nfa::State* e : s->epsilon)
            out << "  s" << s->id << " -> s" << e->id << " [label=\"ε\"];\n";
        for (auto& [ch, dst] : s->transitions)
            out << "  s" << s->id << " -> s" << dst->id
                << " [label=\"" << utils::escape_for_display(ch) << "\"];\n";
        if (s->any_transition)
            out << "  s" << s->id << " -> s" << s->any_transition->id
                << " [label=\".\"];\n";
    }
    out << "}\n";
}

std::string nfa_to_dot(const nfa::NFA& n, const std::string& graph_name)
{
    std::ostringstream oss;
    print_nfa_dot(n, oss, graph_name);
    return oss.str();
}

void print_nfa_text(const nfa::NFA& n, std::ostream& out)
{
    auto states = collect_nfa_states(n);
    out << "NFA: " << states.size() << " states\n";
    for (nfa::State* s : states) {
        out << "  [" << s->id << "]"
            << (s->is_accept ? " ACCEPT" : "")
            << (s == n.start ? " START" : "") << "\n";
        for (nfa::State* e : s->epsilon)
            out << "    --ε--> " << e->id << "\n";
        for (auto& [ch, dst] : s->transitions)
            out << "    --" << utils::escape_for_display(ch) << "--> " << dst->id << "\n";
        if (s->any_transition)
            out << "    --.--> " << s->any_transition->id << "\n";
    }
}

void print_dfa_dot(const dfa::DFA& d, std::ostream& out,
                   const std::string& graph_name)
{
    out << "digraph " << graph_name << " {\n";
    out << "  rankdir=LR;\n";
    out << "  node [shape=circle];\n";

    for (const auto& s : d.states) {
        if (s.is_accept)
            out << "  d" << s.id << " [shape=doublecircle label=\"" << s.id << "\"];\n";
        else
            out << "  d" << s.id << " [label=\"" << s.id << "\"];\n";
    }

    out << "  start [shape=point];\n";
    out << "  start -> d" << d.start << ";\n";

    for (const auto& s : d.states) {
        std::unordered_map<int, std::string> edge_labels;
        for (auto& [ch, dst] : s.transitions) {
            edge_labels[dst] += utils::escape_for_display(ch);
            if (edge_labels[dst].size() > 6)
                edge_labels[dst] = edge_labels[dst].substr(0, 6) + "…";
        }
        for (auto& [dst, label] : edge_labels)
            out << "  d" << s.id << " -> d" << dst
                << " [label=\"" << label << "\"];\n";
    }
    out << "}\n";
}

std::string dfa_to_dot(const dfa::DFA& d, const std::string& graph_name)
{
    std::ostringstream oss;
    print_dfa_dot(d, oss, graph_name);
    return oss.str();
}

void print_dfa_text(const dfa::DFA& d, std::ostream& out)
{
    out << "DFA: " << d.states.size() << " states  start=" << d.start << "\n";
    for (const auto& s : d.states) {
        out << "  [" << s.id << "]"
            << (s.is_accept ? " ACCEPT" : "")
            << (s.id == d.start ? " START" : "") << "\n";
        for (auto& [ch, dst] : s.transitions)
            out << "    --" << utils::escape_for_display(ch) << "--> " << dst << "\n";
    }
}

}  // namespace debug
}  // namespace bishop
