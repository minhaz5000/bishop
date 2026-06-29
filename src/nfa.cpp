#include "bishop/nfa.hpp"

#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "bishop/error.hpp"
#include "bishop/flags.hpp"

namespace nfa {

using std::string;
using std::vector;
using tokenizer::Token;
using tokenizer::TokenType;

static std::vector<State*> collect_states(State* root)
{
    std::vector<State*> result;
    std::unordered_set<State*> visited;
    std::stack<State*> stk;
    stk.push(root);

    while (!stk.empty()) {
        State* s = stk.top();
        stk.pop();
        if (!visited.insert(s).second) continue;
        result.push_back(s);

        for (State* e : s->epsilon) stk.push(e);
        for (auto& [ch, dst] : s->transitions) stk.push(dst);
    }

    return result;
}

static NFA clone_nfa(int& id, const NFA& src)
{
    auto states = collect_states(src.start);

    std::unordered_map<State*, State*> mapping;
    for (State* s : states) {
        State* fresh = new State{id++, s->is_accept};
        mapping[s] = fresh;
    }

    for (State* s : states) {
        State* fresh = mapping[s];
        for (auto& [ch, dst] : s->transitions) fresh->transitions[ch] = mapping[dst];
        for (State* eps : s->epsilon) fresh->epsilon.push_back(mapping[eps]);
        if (s->any_transition) fresh->any_transition = mapping[s->any_transition];
    }

    return {mapping[src.start], mapping[src.end]};
}

NFA create_nfa_repeat(int& id, const NFA& base, int min_rep, int max_rep)
{
    NFA result = clone_nfa(id, base);

    for (int i = 1; i < min_rep; ++i) result = create_nfa_concat(id, result, clone_nfa(id, base));

    if (max_rep < 0) {
        result = create_nfa_concat(id, result, create_nfa_kleene_star(id, clone_nfa(id, base)));
    } else {
        for (int i = min_rep; i < max_rep; ++i)
            result = create_nfa_concat(id, result, create_nfa_question(id, clone_nfa(id, base)));
    }

    return result;
}

NFA create_nfa_group(int& id, NFA inner, int group_idx)
{
    State* open = new State{id++, false};
    State* close = new State{id++, false};
    open->group_start = group_idx;
    close->group_end = group_idx;
    open->epsilon.push_back(inner.start);
    inner.end->is_accept = false;
    inner.end->epsilon.push_back(close);
    close->epsilon.push_back(inner.end);
    return {open, inner.end};
}

NFA parse_postfix(const std::vector<Token>& postfix)
{
    if (postfix.empty()) return NFA{nullptr, nullptr};

    std::stack<NFA> st;
    int state_count = 0;

    for (const auto& tok : postfix) {
        switch (tok.type) {
            case TokenType::Literal:
                st.push(create_nfa_literal(state_count, tok.value));
                break;
            case TokenType::Dot:
                st.push(create_nfa_dot(state_count));
                break;
            case TokenType::WordBoundary: {
                State* s = new State{state_count++, true};
                s->boundary = BoundaryType::WordBoundary;
                st.push(NFA{s, s});
                break;
            }
            case TokenType::NonWordBoundary: {
                State* s = new State{state_count++, true};
                s->boundary = BoundaryType::NonWordBoundary;
                st.push(NFA{s, s});
                break;
            }
            case TokenType::CharClass:
                st.push(create_nfa_charclass(state_count, tok.char_set));
                break;
            case TokenType::Star: {
                if (st.empty()) throw bishop::ParseError("missing operand for '*'");
                NFA prev = st.top();
                st.pop();
                st.push(create_nfa_kleene_star(state_count, prev));
                break;
            }
            case TokenType::Plus: {
                if (st.empty()) throw bishop::ParseError("missing operand for '+'");
                NFA prev = st.top();
                st.pop();
                st.push(create_nfa_plus(state_count, prev));
                break;
            }
            case TokenType::Question: {
                if (st.empty()) throw bishop::ParseError("missing operand for '?'");
                NFA prev = st.top();
                st.pop();
                st.push(create_nfa_question(state_count, prev));
                break;
            }
            case TokenType::Repeat: {
                if (st.empty()) throw bishop::ParseError("missing operand for '{n,m}'");
                NFA prev = st.top();
                st.pop();
                st.push(create_nfa_repeat(state_count, prev, tok.repeat_min, tok.repeat_max));
                break;
            }
            case TokenType::Lookahead: {
                if (st.empty()) throw bishop::ParseError("missing operand for '(?=...)'");
                NFA inner = st.top();
                st.pop();
                st.push(create_nfa_lookahead(state_count, inner, false));
                break;
            }
            case TokenType::NegLookahead: {
                if (st.empty()) throw bishop::ParseError("missing operand for '(?!...)'");
                NFA inner = st.top();
                st.pop();
                st.push(create_nfa_lookahead(state_count, inner, true));
                break;
            }
            case TokenType::Lazy: {
                if (!st.empty()) {
                    NFA top = st.top();
                    st.pop();
                    st.push(create_nfa_lazy_star(state_count, top));
                }
                break;
            }
            case TokenType::Union: {
                if (st.size() < 2) throw bishop::ParseError("missing operand for '|'");
                NFA second = st.top();
                st.pop();
                NFA first = st.top();
                st.pop();
                st.push(create_nfa_union(state_count, first, second));
                break;
            }
            case TokenType::Concat: {
                if (st.size() < 2) throw bishop::ParseError("missing operand for concatenation");
                NFA second = st.top();
                st.pop();
                NFA first = st.top();
                st.pop();
                st.push(create_nfa_concat(state_count, first, second));
                break;
            }
            default:
                throw bishop::ParseError("unknown token in postfix stream");
        }
    }

    if (st.size() != 1) {
        throw bishop::ParseError("malformed postfix expression (stack size " + std::to_string(st.size()) + " at end)");
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
            if (closure.insert(next).second) stk.push(next);
        }
    }
    return closure;
}

static bool is_word(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

static std::set<State*> boundary_filter(const std::set<State*>& states, char prev, char curr_c)
{
    std::set<State*> result;
    bool prev_word = is_word(prev);
    bool curr_word = is_word(curr_c);
    bool at_boundary = prev_word != curr_word;

    for (State* s : states) {
        if (s->boundary == BoundaryType::WordBoundary && !at_boundary) continue;
        if (s->boundary == BoundaryType::NonWordBoundary && at_boundary) continue;
        result.insert(s);
    }
    return result;
}

bool nfa_match(const NFA& nfa, const std::string& input, bishop::Flag flags)
{
    std::set<State*> current = epsilon_closure({nfa.start});

    for (char c : input) {
        std::set<State*> next;
        bool dotall = bishop::has_flag(flags, bishop::Flag::DOTALL);
        for (State* s : current) {
            auto it = s->transitions.find(c);
            if (it != s->transitions.end()) next.insert(it->second);
            if (s->any_transition && (dotall || c != '\n')) next.insert(s->any_transition);
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
    State* end = new State{id++, true};
    start->transitions[ch] = end;
    return {start, end};
}

NFA create_nfa_dot(int& id)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};
    start->any_transition = end;
    return {start, end};
}

NFA create_nfa_charclass(int& id, const std::vector<bool>& charset)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};

    char buf[128];
    int n = 0;
    for (int i = 0; i < 256; ++i) {
        if (charset[i]) buf[n++] = static_cast<char>(i);
    }
    for (int i = 0; i < n; ++i) start->transitions[static_cast<unsigned char>(buf[i])] = end;

    return {start, end};
}

NFA create_nfa_union(int& id, NFA first, NFA second)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};
    start->epsilon.push_back(first.start);
    start->epsilon.push_back(second.start);
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
    State* end = new State{id++, true};
    start->epsilon.push_back(first.start);
    first.end->is_accept = false;
    first.end->epsilon.push_back(first.start);
    first.end->epsilon.push_back(end);
    return {start, end};
}

NFA create_nfa_question(int& id, NFA first)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};
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

NFA create_nfa_lookahead(int& id, NFA inner, bool negative)
{
    State* s = new State{id++, true};
    s->look = negative ? LookType::NegAhead : LookType::Ahead;
    s->look_nfa = new NFA{inner.start, inner.end};
    return {s, s};
}

NFA create_nfa_lazy_star(int& id, NFA first)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};
    start->epsilon.push_back(end);
    start->epsilon.push_back(first.start);
    first.end->is_accept = false;
    first.end->epsilon.push_back(end);
    first.end->epsilon.push_back(first.start);
    return {start, end};
}

NFA create_nfa_lazy_plus(int& id, NFA first)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};
    start->epsilon.push_back(first.start);
    first.end->is_accept = false;
    first.end->epsilon.push_back(end);
    first.end->epsilon.push_back(first.start);
    return {start, end};
}

NFA create_nfa_lazy_question(int& id, NFA first)
{
    State* start = new State{id++, false};
    State* end = new State{id++, true};
    start->epsilon.push_back(end);
    start->epsilon.push_back(first.start);
    first.end->is_accept = false;
    first.end->epsilon.push_back(end);
    return {start, end};
}
}  // namespace nfa

namespace {
using Caps = std::vector<std::pair<size_t, size_t>>;

struct CThread {
    nfa::State* s;
    Caps caps;
};

static void eps_expand(std::vector<CThread>& dest, nfa::State* st, Caps caps, size_t pos,
                       std::unordered_set<nfa::State*>& visited, int max_group)
{
    if (!visited.insert(st).second) return;

    if (st->group_start >= 0 && st->group_start < max_group) caps[static_cast<size_t>(st->group_start)].first = pos;
    if (st->group_end >= 0 && st->group_end < max_group) caps[static_cast<size_t>(st->group_end)].second = pos;

    dest.push_back({st, caps});

    for (nfa::State* e : st->epsilon) eps_expand(dest, e, caps, pos, visited, max_group);
}

static int max_group_count(nfa::State* root)
{
    int mx = 0;
    std::unordered_set<nfa::State*> visited;
    std::stack<nfa::State*> stk;
    stk.push(root);
    while (!stk.empty()) {
        nfa::State* s = stk.top();
        stk.pop();
        if (!visited.insert(s).second) continue;
        if (s->group_start >= 0) mx = std::max(mx, s->group_start + 1);
        if (s->group_end >= 0) mx = std::max(mx, s->group_end + 1);
        for (nfa::State* e : s->epsilon) stk.push(e);
        for (auto& [c, d] : s->transitions) stk.push(d);
        if (s->any_transition) stk.push(s->any_transition);
    }
    return mx;
}
}  // namespace

namespace nfa {

MatchData nfa_match_captures(const NFA& nfa, const std::string& input, size_t from, bishop::Flag flags)
{
    MatchData result;
    if (!nfa.start) return result;

    const int max_g = max_group_count(nfa.start);

    auto init_caps = [&]() { return Caps(static_cast<size_t>(max_g), {std::string::npos, std::string::npos}); };

    const bool dotall = bishop::has_flag(flags, bishop::Flag::DOTALL);

    std::vector<CThread> cur, nxt;
    cur.reserve(64);
    nxt.reserve(64);

    {
        std::unordered_set<State*> vis;
        eps_expand(cur, nfa.start, init_caps(), from, vis, max_g);
    }

    for (size_t i = from; i <= input.size(); ++i) {
        for (const CThread& t : cur) {
            if (t.s->is_accept) {
                if (!result.matched || i > result.end) {
                    result.matched = true;
                    result.start = from;
                    result.end = i;
                    result.groups = t.caps;
                }
            }
        }
        if (i == input.size()) break;

        const char c = input[i];
        nxt.clear();

        for (const CThread& t : cur) {
            auto advance = [&](State* dst) {
                std::unordered_set<State*> vis;
                eps_expand(nxt, dst, t.caps, i + 1, vis, max_g);
            };

            auto it = t.s->transitions.find(c);
            if (it != t.s->transitions.end()) advance(it->second);

            if (t.s->any_transition && (dotall || c != '\n')) advance(t.s->any_transition);
        }

        cur.swap(nxt);
        if (cur.empty()) break;
    }

    return result;
}

}  // namespace nfa
