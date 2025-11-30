#ifndef BISHOP_FLAGS_HPP_
#define BISHOP_FLAGS_HPP_

#include <cstdint>
#include <string>

namespace bishop {

enum class Flag : uint32_t {
    NONE        = 0,
    IGNORECASE  = 1 << 0,
    MULTILINE   = 1 << 1,
    DOTALL      = 1 << 2,
    EXTENDED    = 1 << 3,
    GLOBAL      = 1 << 4,
};

inline Flag operator|(Flag a, Flag b) {
    return static_cast<Flag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline Flag operator&(Flag a, Flag b) {
    return static_cast<Flag>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline Flag operator~(Flag a) {
    return static_cast<Flag>(~static_cast<uint32_t>(a));
}

inline Flag& operator|=(Flag& a, Flag b) { a = a | b; return a; }
inline Flag& operator&=(Flag& a, Flag b) { a = a & b; return a; }

inline bool has_flag(Flag flags, Flag f) {
    return (flags & f) != Flag::NONE;
}

inline std::string flags_to_string(Flag f) {
    std::string s;
    if (has_flag(f, Flag::IGNORECASE)) s += 'i';
    if (has_flag(f, Flag::MULTILINE))  s += 'm';
    if (has_flag(f, Flag::DOTALL))     s += 's';
    if (has_flag(f, Flag::EXTENDED))   s += 'x';
    if (has_flag(f, Flag::GLOBAL))     s += 'g';
    return s;
}

inline Flag flags_from_string(const std::string& s) {
    Flag f = Flag::NONE;
    for (char c : s) {
        switch (c) {
            case 'i': f |= Flag::IGNORECASE; break;
            case 'm': f |= Flag::MULTILINE;  break;
            case 's': f |= Flag::DOTALL;     break;
            case 'x': f |= Flag::EXTENDED;   break;
            case 'g': f |= Flag::GLOBAL;     break;
            default: break;
        }
    }
    return f;
}

}  // namespace bishop
#endif
