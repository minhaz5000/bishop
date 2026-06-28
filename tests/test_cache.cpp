#include "bishop/cache.hpp"
#include "bishop/nfa.hpp"
#include "bishop/tokenizer.hpp"
#include "framework.hpp"

using bishop::Flag;
using bishop::cache::LRUCache;
using bishop::cache::PatternCache;

static nfa::NFA compile_nfa(const std::string& pat)
{
    auto t = tokenizer::tokenize(pat);
    tokenizer::strip_anchors(t);
    t = tokenizer::insert_concats(t);
    return nfa::parse_postfix(tokenizer::to_postfix(t));
}

TEST(lru_cache_put_get)
{
    LRUCache<std::string, int> c(4);
    c.put("a", 1);
    int v = 0;
    ASSERT_TRUE(c.get("a", v));
    ASSERT_EQ(v, 1);
}

TEST(lru_cache_miss)
{
    LRUCache<std::string, int> c(4);
    int v = 0;
    ASSERT_FALSE(c.get("x", v));
}

TEST(lru_cache_eviction)
{
    LRUCache<std::string, int> c(2);
    c.put("a", 1);
    c.put("b", 2);
    c.put("c", 3);
    int v = 0;
    ASSERT_FALSE(c.get("a", v));
    ASSERT_TRUE(c.get("b", v));
    ASSERT_TRUE(c.get("c", v));
}

TEST(lru_cache_lru_order)
{
    LRUCache<std::string, int> c(2);
    c.put("a", 1);
    c.put("b", 2);
    int v = 0;
    c.get("a", v);
    c.put("c", 3);
    ASSERT_TRUE(c.get("a", v));
    ASSERT_FALSE(c.get("b", v));
}

TEST(lru_cache_size)
{
    LRUCache<std::string, int> c(8);
    c.put("x", 1);
    c.put("y", 2);
    ASSERT_EQ(c.size(), 2u);
}

TEST(lru_cache_contains)
{
    LRUCache<std::string, int> c(8);
    c.put("k", 42);
    ASSERT_TRUE(c.contains("k"));
    ASSERT_FALSE(c.contains("z"));
}

TEST(lru_cache_clear)
{
    LRUCache<std::string, int> c(8);
    c.put("a", 1);
    c.put("b", 2);
    c.clear();
    ASSERT_EQ(c.size(), 0u);
}

TEST(lru_cache_resize)
{
    LRUCache<std::string, int> c(8);
    c.put("a", 1);
    c.put("b", 2);
    c.put("c", 3);
    c.resize(2);
    ASSERT_TRUE(c.size() <= 2u);
}

TEST(pattern_cache_put_get_nfa)
{
    PatternCache pc(32);
    auto n = compile_nfa("abc");
    pc.put_nfa("abc", Flag::NONE, n);
    nfa::NFA out{nullptr, nullptr};
    ASSERT_TRUE(pc.get_nfa("abc", Flag::NONE, out));
    ASSERT_NE(out.start, nullptr);
}

TEST(pattern_cache_miss)
{
    PatternCache pc(32);
    nfa::NFA out{nullptr, nullptr};
    ASSERT_FALSE(pc.get_nfa("xyz", Flag::NONE, out));
}

TEST(pattern_cache_hit_rate)
{
    PatternCache pc(32);
    auto n = compile_nfa("a+");
    pc.put_nfa("a+", Flag::NONE, n);
    nfa::NFA out{nullptr, nullptr};
    pc.get_nfa("a+", Flag::NONE, out);
    pc.get_nfa("b+", Flag::NONE, out);
    ASSERT_EQ(pc.hits(), 1u);
    ASSERT_EQ(pc.misses(), 1u);
    ASSERT_EQ(pc.hit_rate(), 0.5);
}

TEST(pattern_cache_clear_resets_stats)
{
    PatternCache pc(32);
    auto n = compile_nfa("x");
    pc.put_nfa("x", Flag::NONE, n);
    nfa::NFA out{nullptr, nullptr};
    pc.get_nfa("x", Flag::NONE, out);
    pc.clear();
    ASSERT_EQ(pc.size(), 0u);
    ASSERT_EQ(pc.hits(), 0u);
    ASSERT_EQ(pc.misses(), 0u);
}

TEST(pattern_cache_flags_differentiate)
{
    PatternCache pc(32);
    auto n = compile_nfa("hello");
    pc.put_nfa("hello", Flag::NONE, n);
    pc.put_nfa("hello", Flag::IGNORECASE, n);
    nfa::NFA out{nullptr, nullptr};
    ASSERT_TRUE(pc.get_nfa("hello", Flag::NONE, out));
    ASSERT_TRUE(pc.get_nfa("hello", Flag::IGNORECASE, out));
}

TEST(global_cache_accessible)
{
    auto& gc = bishop::cache::global_cache();
    ASSERT_TRUE(gc.capacity() > 0u);
}
