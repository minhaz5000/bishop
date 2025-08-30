#include "bishop/nfa.hpp"

#include <cstdlib>
#include <set>
#include <stack>

namespace nfa {

using std::string;
using std::vector;
using tokenizer::Token;
using tokenizer::TokenType;

NFA parse_postfix(const std::vector<Token>& postfix)
{
    if (postfix.empty()) return NFA{nullptr, nullptr};

    std::stack<NFA> st;
    int state_count = 0;

    for (auto& tok : postfix) {
        switch (tok.type) {
            case TokenType::Literal:
                st.push(create_nfa_literal(state_count, tok.value));
                break;

            case TokenType::Dot:
                st.push(create_nfa_dot(state_count));
                break;

            case TokenType::Star: {
                NFA prev = st.top(); st.pop();
                st.push(create_nfa_kleene_star(state_count, prev));
                break;
            }
            case TokenType::Plus: {
                NFA prev = st.top(); st.pop();
                st.push(create_nfa_plus(state_count, prev));
                break;
            }
            case TokenType::Question: {
                NFA prev = st.top(); st.pop();
                st.push(create_nfa_question(state_count, prev));
                break;
            }
            case TokenType::Union: {
                NFA second = st.top(); st.pop();
                NFA first  = st.top(); st.pop();
                st.push(create_nfa_union(state_count, first, second));
                break;
            }
            case TokenType::Concat: {
                NFA second = st.top(); st.pop();
                NFA first  = st.top(); st.pop();
                st.push(create_nfa_concat(state_count, first, second));
                break;
            }
            default:
                std::cerr << "unknown token\n";
                exit(EXIT_FAILURE);
        }
    }

    if (st.size() != 1) {
        std::cerr << "invalid postfix\n";
        exit(EXIT_FAILURE);
    }

    return st.top();
}

std::set<State*> epsilon_closure(const std::set<State*>& states)
{
    std::set<State*> closure = states;
    std::stack<State*> stk;
    for (State* s : states) stk.push(s);

    while (!stk.empty()) {
        State* curr = stk.top(); stk.pop();
        for (State* next : curr->epsilon) {
            if (closure.insert(next).second)
                stk.push(next);
        }
    }
    return closure;
}

bool nfa_match(NFA& nfa, const std::string& input)
{
    std::set<State*> current = epsilon_closure({nfa.start});

    for (char c : input) {
        std::set<State*> next;
        for (State* s : current) {
            auto it = s->transitions.find(c);
            if (it != s->transitions.end())
                next.insert(it->second);
            if (s->any_transition)
                next.insert(s->any_transition);
        }
        current = epsilon_closure(next);
        if (current.empty()) return false;
    }

    for (State* s : current) {
        if (s->is_accept) return true;
    }
    return false;
}

NFA create_nfa_literal(int& id, char ch)
{
    State* start = new State{id++, false};
    State* end   = new State{id++, true};
    start->transitions[ch] = end;
    return {start, end};
}

NFA create_nfa_dot(int& id)
{
    State* start = new State{id++, false};
    State* end   = new State{id++, true};
    start->any_transition = end;
    return {start, end};
}

NFA create_nfa_union(int& id, NFA first, NFA second)
{
    State* start = new State{id++, false};
    State* end   = new State{id++, true};
    start->epsilon.push_back(first.start);
    start->epsilon.push_back(second.start);
    first.end->is_accept  = false;
    second.end->is_accept = false;
    first.end->epsilon.push_back(end);
    second.end->epsilon.push_back(end);
    return {start, end};
}

NFA create_nfa_kleene_star(int& id, NFA first)
{
    State* start = new State{id++, false};
    State* end   = new State{id++, true};
    start->epsilon.push_back(first.start);
    start->epsilon.push_back(end);
    first.end->is_accept = false;
    first.end->epsilon.push_back(first.start);
    first.end->epsilon.push_back(end);
    return {start, end};
}

NFA create_nfa_plus(int& id, NFA first)
{
    State* start = new State{id++, false};
    State* end   = new State{id++, true};
    start->epsilon.push_back(first.start);
    first.end->is_accept = false;
    first.end->epsilon.push_back(first.start);
    first.end->epsilon.push_back(end);
    return {start, end};
}

NFA create_nfa_question(int& id, NFA first)
{
    State* start = new State{id++, false};
    State* end   = new State{id++, true};
    start->epsilon.push_back(first.start);
    start->epsilon.push_back(end);
    first.end->is_accept = false;
    first.end->epsilon.push_back(end);
    return {start, end};
}

NFA create_nfa_concat(int& id, NFA first, NFA second)
{
    first.end->is_accept = false;
    first.end->epsilon.push_back(second.start);
    return {first.start, second.end};
}

}  // namespace nfa
