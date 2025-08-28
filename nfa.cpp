#include "nfa.hpp"

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

            case TokenType::Star: {
                NFA prev = st.top();
                st.pop();
                st.push(create_nfa_kleene_star(state_count, prev));
                break;
            }

            case TokenType::Plus: {
                NFA prev = st.top();
                st.pop();
                st.push(create_nfa_plus(state_count, prev));
                break;
            }

            case TokenType::Question: {
                NFA prev = st.top();
                st.pop();
                st.push(create_nfa_question(state_count, prev));
                break;
            }

            case TokenType::Union: {
                NFA second = st.top();
                st.pop();  // right operand
                NFA first = st.top();
                st.pop();  // left operand
                st.push(create_nfa_union(state_count, first, second));
                break;
            }

            case TokenType::Concat: {
                NFA second = st.top();
                st.pop();  // right operand
                NFA first = st.top();
                st.pop();  // left operand
                st.push(create_nfa_concat(state_count, first, second));
                break;
            }

            default:
                std::cerr << "Error: unknown token type\n";
                exit(EXIT_FAILURE);
        }
    }

    if (st.size() != 1) {
        std::cerr << "Error: postfix expression invalid\n";
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
        State* curr = stk.top();
        stk.pop();

        for (State* next : curr->epsilon) {
            if (closure.insert(next).second) {  // newly inserted
                stk.push(next);
            }
        }
    }

    return closure;
}

bool nfa_match(NFA& nfa, const std::string& input)
{
    // Step 1: initial states (epsilon closure of start)
    std::set<State*> current = epsilon_closure({nfa.start});

    // Step 2: process each character
    for (char c : input) {
        std::set<State*> next;

        // Move on character transitions
        for (State* s : current) {
            if (s->transitions.count(c)) {
                next.insert(s->transitions[c]);
            }
        }

        // Take epsilon closure of the next set
        current = epsilon_closure(next);

        // No states reachable → fail early
        if (current.empty()) return false;
    }

    // Step 3: check if any accepting state is reached
    for (State* s : current) {
        if (s->is_accept) return true;
    }

    return false;
}

NFA create_nfa_literal(int& id, char ch)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};

    start->transitions[ch] = end;
    return {start, end};
}

NFA create_nfa_union(int& id, NFA first, NFA second)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};

    // new start has epsilon to both NFAs
    start->epsilon.push_back(first.start);
    start->epsilon.push_back(second.start);

    // both old ends go to new end via epsilon
    first.end->is_accept = false;
    second.end->is_accept = false;
    first.end->epsilon.push_back(end);
    second.end->epsilon.push_back(end);

    return {start, end};
}

NFA create_nfa_kleene_star(int& id, NFA first)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};

    // new start has epsilon to old start and new end
    start->epsilon.push_back(first.start);
    start->epsilon.push_back(end);

    // old end has epsilon to old start and new end
    first.end->is_accept = false;
    first.end->epsilon.push_back(first.start);
    first.end->epsilon.push_back(end);

    return {start, end};
}

NFA create_nfa_plus(int& id, NFA first)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};

    // new start must go into the first NFA
    start->epsilon.push_back(first.start);

    // old end loops back and can also go to new end
    first.end->is_accept = false;
    first.end->epsilon.push_back(first.start);
    first.end->epsilon.push_back(end);

    return {start, end};
}

NFA create_nfa_question(int& id, NFA first)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};

    // new start has epsilon to old start and new end
    start->epsilon.push_back(first.start);
    start->epsilon.push_back(end);

    // old end has epsilon to new end
    first.end->is_accept = false;
    first.end->epsilon.push_back(end);

    return {start, end};
}

NFA create_nfa_concat(int& id, NFA first, NFA second)
{
    // join the end of first to the start of second
    first.end->is_accept = false;
    first.end->epsilon.push_back(second.start);

    return {first.start, second.end};
}

}  // namespace nfa
