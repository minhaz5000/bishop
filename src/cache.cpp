#include "bishop/cache.hpp"

namespace bishop {
namespace cache {

PatternCache::PatternCache(size_t cap) : cache_(cap) {}

bool PatternCache::get_nfa(const std::string& pattern, Flag flags,
                            nfa::NFA& out)
{
    std::lock_guard<std::mutex> lk(mu_);
    std::string key = make_key(pattern, flags);
    CompiledEntry e;
    if (!cache_.get(key, e)) { ++misses_; return false; }
    ++hits_;
    out = e.nfa;
    return true;
}

void PatternCache::put_nfa(const std::string& pattern, Flag flags,
                            const nfa::NFA& n)
{
    std::lock_guard<std::mutex> lk(mu_);
    std::string key = make_key(pattern, flags);
    CompiledEntry e;
    cache_.get(key, e);
    e.nfa = n;
    cache_.put(key, e);
}

bool PatternCache::get_dfa(const std::string& pattern, Flag flags,
                            dfa::DFA& out)
{
    std::lock_guard<std::mutex> lk(mu_);
    std::string key = make_key(pattern, flags);
    CompiledEntry e;
    if (!cache_.get(key, e) || !e.dfa_ready) { ++misses_; return false; }
    ++hits_;
    out = e.dfa;
    return true;
}

void PatternCache::put_dfa(const std::string& pattern, Flag flags,
                            const dfa::DFA& d)
{
    std::lock_guard<std::mutex> lk(mu_);
    std::string key = make_key(pattern, flags);
    CompiledEntry e;
    cache_.get(key, e);
    e.dfa       = d;
    e.dfa_ready = true;
    cache_.put(key, e);
}

size_t PatternCache::size()     const noexcept { return cache_.size(); }
size_t PatternCache::capacity() const noexcept { return cache_.capacity(); }
void   PatternCache::clear()                   { cache_.clear(); hits_ = misses_ = 0; }
void   PatternCache::resize(size_t c)          { cache_.resize(c); }

double PatternCache::hit_rate() const noexcept {
    size_t total = hits_ + misses_;
    return total ? static_cast<double>(hits_) / total : 0.0;
}

PatternCache& global_cache()
{
    static PatternCache inst(256);
    return inst;
}

}  // namespace cache
}  // namespace bishop
