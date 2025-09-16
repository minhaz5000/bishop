#ifndef BISHOP_H_
#define BISHOP_H_

#include <string>
#include <utility>
#include <vector>

#include "bishop/nfa.hpp"
#include "bishop/tokenizer.hpp"

namespace bishop {

class Regex {
public:
    Regex() = default;

    explicit Regex(const std::string& pattern) { compile(pattern); }

    void compile(const std::string& pattern) {
        auto tokens        = tokenizer::tokenize(pattern);
        auto [anc_s, anc_e] = tokenizer::strip_anchors(tokens);
        anchored_start_    = anc_s;
        anchored_end_      = anc_e;
        tokens             = tokenizer::insert_concats(tokens);
        auto postfix       = tokenizer::to_postfix(tokens);
        nfa_               = nfa::parse_postfix(postfix);
    }

    bool match(const std::string& text) {
        return nfa::nfa_match(nfa_, text);
    }

    std::pair<size_t, size_t> search(const std::string& text) {
        size_t start_lo = 0;
        size_t start_hi = anchored_start_ ? 1 : text.size() + 1;

        for (size_t i = start_lo; i < start_hi; ++i) {
            size_t j_lo = anchored_end_ ? text.size() : i;
            for (size_t j = j_lo; j <= text.size(); ++j) {
                if (nfa::nfa_match(nfa_, text.substr(i, j - i)))
                    return {i, j};
            }
        }
        return {std::string::npos, std::string::npos};
    }

    std::vector<std::pair<size_t, size_t>> find_all(const std::string& text) {
        std::vector<std::pair<size_t, size_t>> results;
        size_t pos = 0;
        while (pos <= text.size()) {
            bool found = false;
            for (size_t j = text.size(); j >= pos; --j) {
                if (nfa::nfa_match(nfa_, text.substr(pos, j - pos))) {
                    results.push_back({pos, j});
                    pos = (j == pos) ? j + 1 : j;
                    found = true;
                    break;
                }
                if (j == pos) break;
            }
            if (!found) ++pos;
        }
        return results;
    }

    std::string replace(const std::string& text, const std::string& repl) {
        auto [s, e] = search(text);
        if (s == std::string::npos) return text;
        return text.substr(0, s) + repl + text.substr(e);
    }

    std::string replace_all(const std::string& text, const std::string& repl) {
        auto matches = find_all(text);
        if (matches.empty()) return text;
        std::string result;
        size_t pos = 0;
        for (auto& [s, e] : matches) {
            result += text.substr(pos, s - pos);
            result += repl;
            pos = e;
        }
        result += text.substr(pos);
        return result;
    }

    bool anchored_start() const { return anchored_start_; }
    bool anchored_end()   const { return anchored_end_; }

private:
    nfa::NFA nfa_{nullptr, nullptr};
    bool     anchored_start_{false};
    bool     anchored_end_{false};
};

}  // namespace bishop
#endif
