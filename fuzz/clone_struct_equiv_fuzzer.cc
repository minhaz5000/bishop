#include <cstdint>
#include <cstdlib>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "bishop/error.hpp"
#include "bishop/flags.hpp"
#include "bishop/nfa.hpp"
#include "bishop/tokenizer.hpp"

namespace {

bool try_compile(const std::string& pattern, bishop::Flag flags, nfa::NFA& out)
{
    try {
        auto tokens = tokenizer::tokenize(pattern, flags);
        tokenizer::strip_anchors(tokens);
        tokens = tokenizer::insert_concats(tokens);
        auto postfix = tokenizer::to_postfix(tokens);
        out = nfa::parse_postfix(postfix);
        return true;
    }
    catch (...) {
        return false;
    }
}

void assert_equivalent(nfa::State* a, nfa::State* b)
{
    if ((a == nullptr) != (b == nullptr)) std::abort();
    if (a == nullptr) return;

    std::unordered_map<nfa::State*, nfa::State*> mapping;
    std::unordered_set<nfa::State*> visited_b;
    std::stack<std::pair<nfa::State*, nfa::State*>> stk;
    stk.push({a, b});

    while (!stk.empty()) {
        auto [sa, sb] = stk.top();
        stk.pop();

        auto it = mapping.find(sa);
        if (it != mapping.end()) {
            if (it->second != sb) std::abort();
            continue;
        }
        if (!visited_b.insert(sb).second) std::abort();
        mapping[sa] = sb;

        if (sa->is_accept != sb->is_accept) std::abort();
        if (sa->look != sb->look) std::abort();
        if (sa->transitions.size() != sb->transitions.size()) std::abort();
        if (sa->epsilon.size() != sb->epsilon.size()) std::abort();
        if ((sa->any_transition == nullptr) != (sb->any_transition == nullptr)) std::abort();
        if ((sa->look_nfa == nullptr) != (sb->look_nfa == nullptr)) {
            std::abort();
        }

        for (auto& [ch, dst] : sa->transitions) {
            auto bit = sb->transitions.find(ch);
            if (bit == sb->transitions.end()) std::abort();
            stk.push({dst, bit->second});
        }
        for (size_t i = 0; i < sa->epsilon.size(); ++i) stk.push({sa->epsilon[i], sb->epsilon[i]});
        if (sa->any_transition) stk.push({sa->any_transition, sb->any_transition});

        if (sa->look_nfa) {
            assert_equivalent(sa->look_nfa->start, sb->look_nfa->start);
        }
    }
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 2) return 0;

    const uint8_t flag_byte = data[0];
    bishop::Flag flags = bishop::Flag::NONE;
    if (flag_byte & 0x01) flags |= bishop::Flag::IGNORECASE;
    if (flag_byte & 0x02) flags |= bishop::Flag::MULTILINE;
    if (flag_byte & 0x04) flags |= bishop::Flag::DOTALL;

    static constexpr size_t kMaxPattern = 256;
    const std::string pattern(reinterpret_cast<const char*>(data + 1), std::min(size - 1, kMaxPattern));

    nfa::NFA original{nullptr, nullptr};
    if (!try_compile(pattern, flags, original)) return 0;

    int id = 100000;
    nfa::NFA clone = nfa::clone_nfa(id, original);

    assert_equivalent(original.start, clone.start);

    return 0;
}
