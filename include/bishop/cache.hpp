#ifndef BISHOP_CACHE_HPP_
#define BISHOP_CACHE_HPP_

#include <list>
#include <mutex>
#include <string>
#include <unordered_map>

#include "bishop/dfa.hpp"
#include "bishop/flags.hpp"
#include "bishop/nfa.hpp"

namespace bishop {
namespace cache {

template <typename K, typename V>
class LRUCache {
public:
    explicit LRUCache(size_t cap = 64) : cap_(cap) {}

    bool get(const K& key, V& out)
    {
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        order_.splice(order_.begin(), order_, it->second);
        out = it->second->second;
        return true;
    }

    void put(const K& key, V val)
    {
        auto it = map_.find(key);
        if (it != map_.end()) {
            it->second->second = std::move(val);
            order_.splice(order_.begin(), order_, it->second);
            return;
        }
        if (order_.size() >= cap_) {
            map_.erase(order_.back().first);
            order_.pop_back();
        }
        order_.emplace_front(key, std::move(val));
        map_[key] = order_.begin();
    }

    bool contains(const K& key) const
    {
        return map_.count(key) > 0;
    }
    size_t size() const noexcept
    {
        return map_.size();
    }
    size_t capacity() const noexcept
    {
        return cap_;
    }
    void clear()
    {
        map_.clear();
        order_.clear();
    }

    void resize(size_t new_cap)
    {
        cap_ = new_cap;
        while (order_.size() > cap_) {
            map_.erase(order_.back().first);
            order_.pop_back();
        }
    }

private:
    using List = std::list<std::pair<K, V>>;
    size_t cap_;
    List order_;
    std::unordered_map<K, typename List::iterator> map_;
};

struct CacheKey {
    std::string pattern;
    uint32_t flags{0};

    bool operator==(const CacheKey& o) const noexcept
    {
        return pattern == o.pattern && flags == o.flags;
    }
};

struct CacheKeyHash {
    size_t operator()(const CacheKey& k) const noexcept
    {
        return std::hash<std::string>{}(k.pattern) ^ (k.flags * 2654435761u);
    }
};

struct CompiledEntry {
    nfa::NFA nfa;
    dfa::DFA dfa;
    bool dfa_ready{false};
};

class PatternCache {
public:
    explicit PatternCache(size_t cap = 128);

    bool get_nfa(const std::string& pattern, Flag flags, nfa::NFA& out);
    void put_nfa(const std::string& pattern, Flag flags, const nfa::NFA& n);

    bool get_dfa(const std::string& pattern, Flag flags, dfa::DFA& out);
    void put_dfa(const std::string& pattern, Flag flags, const dfa::DFA& d);

    size_t size() const noexcept;
    size_t capacity() const noexcept;
    void clear();
    void resize(size_t new_cap);

    size_t hits() const noexcept
    {
        return hits_;
    }
    size_t misses() const noexcept
    {
        return misses_;
    }
    double hit_rate() const noexcept;

private:
    using Map = LRUCache<std::string, CompiledEntry>;
    mutable std::mutex mu_;
    Map cache_;
    mutable size_t hits_{0};
    mutable size_t misses_{0};

    static std::string make_key(const std::string& p, Flag f)
    {
        return p + '\0' + std::to_string(static_cast<uint32_t>(f));
    }
};

PatternCache& global_cache();

}  // namespace cache
}  // namespace bishop

namespace std {
template <>
struct hash<bishop::cache::CacheKey> : bishop::cache::CacheKeyHash {};
}  // namespace std
#endif
