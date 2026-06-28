#pragma once
#include <cstdio>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& test_registry()
{
    static std::vector<TestCase> reg;
    return reg;
}

struct TestRegistrar {
    TestRegistrar(const char* name, std::function<void()> fn)
    {
        test_registry().push_back({name, fn});
    }
};

#define TEST(name)                                         \
    static void _test_##name();                            \
    static TestRegistrar _reg_##name(#name, _test_##name); \
    static void _test_##name()

#define ASSERT_TRUE(x)                                             \
    do {                                                           \
        if (!(x)) throw std::runtime_error("ASSERT_TRUE(" #x ")"); \
    } while (0)

#define ASSERT_FALSE(x)                                          \
    do {                                                         \
        if (x) throw std::runtime_error("ASSERT_FALSE(" #x ")"); \
    } while (0)

#define ASSERT_EQ(a, b)                                                          \
    do {                                                                         \
        if (!((a) == (b))) throw std::runtime_error("ASSERT_EQ: " #a " != " #b); \
    } while (0)

#define ASSERT_NE(a, b)                                                       \
    do {                                                                      \
        if ((a) == (b)) throw std::runtime_error("ASSERT_NE: " #a " == " #b); \
    } while (0)

#define ASSERT_GE(a, b)                                                         \
    do {                                                                        \
        if (!((a) >= (b))) throw std::runtime_error("ASSERT_GE: " #a " < " #b); \
    } while (0)

#define ASSERT_THROWS(expr)                                                               \
    do {                                                                                  \
        bool _threw = false;                                                              \
        try {                                                                             \
            expr;                                                                         \
        }                                                                                 \
        catch (...) {                                                                     \
            _threw = true;                                                                \
        }                                                                                 \
        if (!_threw) throw std::runtime_error("ASSERT_THROWS: no exception for: " #expr); \
    } while (0)

#define ASSERT_NO_THROW(expr)                                                            \
    do {                                                                                 \
        try {                                                                            \
            expr;                                                                        \
        }                                                                                \
        catch (const std::exception& e) {                                                \
            throw std::runtime_error(std::string("ASSERT_NO_THROW threw: ") + e.what()); \
        }                                                                                \
    } while (0)
