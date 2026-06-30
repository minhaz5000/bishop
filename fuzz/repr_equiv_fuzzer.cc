#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include "bishop/dfa.hpp"
#include "bishop/error.hpp"
#include "bishop/flags.hpp"
#include "bishop/nfa.hpp"
#include "bishop/serializer.hpp"
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

std::vector<std::string> build_probes(const std::string& pattern)
{
    std::vector<std::string> probes = {
        "", "a", "ab", "abc", "aaa", "aaab", "ZZZ", "123", std::string(1, '\0'), std::string(16, 'a'),
    };
    probes.push_back(pattern);
    if (pattern.size() > 1) {
        probes.push_back(pattern.substr(0, pattern.size() / 2));
        probes.push_back(pattern.substr(pattern.size() / 2));
    }
    return probes;
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

    static constexpr size_t kMaxPattern = 200;
    const std::string pattern(reinterpret_cast<const char*>(data + 1), std::min(size - 1, kMaxPattern));

    nfa::NFA n{nullptr, nullptr};
    if (!try_compile(pattern, flags, n)) return 0;

    dfa::DFA d;
    try {
        d = dfa::nfa_to_dfa(n);
    }
    catch (...) {
        nfa::free_nfa(n);
        return 0;
    }
    if (!d.valid) {
        nfa::free_nfa(n);
        return 0;
    }

    dfa::DFA min;
    bool have_min = true;
    try {
        min = dfa::minimize(d);
    }
    catch (...) {
        have_min = false;
    }

    dfa::DFA roundtripped;
    bool have_roundtrip = true;
    try {
        std::ostringstream out;
        bishop::serial::serialize_dfa(d, out);
        std::istringstream in(out.str());
        roundtripped = bishop::serial::deserialize_dfa(in);
    }
    catch (...) {
        have_roundtrip = false;
    }

    for (const auto& probe : build_probes(pattern)) {
        bool m_nfa;
        try {
            m_nfa = nfa::nfa_match(n, probe, flags);
        }
        catch (...) {
            continue;
        }

        bool m_dfa = dfa::dfa_match(d, probe);
        if (m_nfa != m_dfa) std::abort();

        if (have_min) {
            bool m_min = dfa::dfa_match(min, probe);
            if (m_dfa != m_min) std::abort();
        }

        if (have_roundtrip && roundtripped.valid) {
            bool m_rt = dfa::dfa_match(roundtripped, probe);
            if (m_dfa != m_rt) std::abort();
        }
    }

    nfa::free_nfa(n);
    return 0;
}
