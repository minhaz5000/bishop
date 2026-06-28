#ifndef BISHOP_POSIX_HPP_
#define BISHOP_POSIX_HPP_

#include <cctype>
#include <string>
#include <vector>

namespace bishop {
namespace posix {

inline std::vector<bool> cls_alpha()
{
    std::vector<bool> cs(256, false);
    for (int i = 0; i < 256; ++i) cs[i] = std::isalpha(i) != 0;
    return cs;
}

inline std::vector<bool> cls_digit()
{
    std::vector<bool> cs(256, false);
    for (int i = 0; i < 256; ++i) cs[i] = std::isdigit(i) != 0;
    return cs;
}

inline std::vector<bool> cls_alnum()
{
    std::vector<bool> cs(256, false);
    for (int i = 0; i < 256; ++i) cs[i] = std::isalnum(i) != 0;
    return cs;
}

inline std::vector<bool> cls_space()
{
    std::vector<bool> cs(256, false);
    for (int i = 0; i < 256; ++i) cs[i] = std::isspace(i) != 0;
    return cs;
}

inline std::vector<bool> cls_upper()
{
    std::vector<bool> cs(256, false);
    for (int i = 0; i < 256; ++i) cs[i] = std::isupper(i) != 0;
    return cs;
}

inline std::vector<bool> cls_lower()
{
    std::vector<bool> cs(256, false);
    for (int i = 0; i < 256; ++i) cs[i] = std::islower(i) != 0;
    return cs;
}

inline std::vector<bool> cls_punct()
{
    std::vector<bool> cs(256, false);
    for (int i = 0; i < 256; ++i) cs[i] = std::ispunct(i) != 0;
    return cs;
}

inline std::vector<bool> cls_print()
{
    std::vector<bool> cs(256, false);
    for (int i = 0; i < 256; ++i) cs[i] = std::isprint(i) != 0;
    return cs;
}

inline std::vector<bool> cls_graph()
{
    std::vector<bool> cs(256, false);
    for (int i = 0; i < 256; ++i) cs[i] = std::isgraph(i) != 0;
    return cs;
}

inline std::vector<bool> cls_cntrl()
{
    std::vector<bool> cs(256, false);
    for (int i = 0; i < 256; ++i) cs[i] = std::iscntrl(i) != 0;
    return cs;
}

inline std::vector<bool> cls_xdigit()
{
    std::vector<bool> cs(256, false);
    for (int i = 0; i < 256; ++i) cs[i] = std::isxdigit(i) != 0;
    return cs;
}

inline std::vector<bool> cls_blank()
{
    std::vector<bool> cs(256, false);
    cs[' '] = cs['\t'] = true;
    return cs;
}

inline std::vector<bool> from_name(const std::string& name)
{
    if (name == "alpha") return cls_alpha();
    if (name == "digit") return cls_digit();
    if (name == "alnum") return cls_alnum();
    if (name == "space") return cls_space();
    if (name == "upper") return cls_upper();
    if (name == "lower") return cls_lower();
    if (name == "punct") return cls_punct();
    if (name == "print") return cls_print();
    if (name == "graph") return cls_graph();
    if (name == "cntrl") return cls_cntrl();
    if (name == "xdigit") return cls_xdigit();
    if (name == "blank") return cls_blank();
    return std::vector<bool>(256, false);
}

}  // namespace posix
}  // namespace bishop
#endif
