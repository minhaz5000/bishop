#include "bishop/utils.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace bishop {
namespace utils {

bool is_word_char(unsigned char c) {
    return std::isalnum(c) || c == '_';
}

bool is_digit_char(unsigned char c) {
    return c >= '0' && c <= '9';
}

bool is_space_char(unsigned char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

std::string escape_for_display(unsigned char c) {
    switch (c) {
        case '\n': return "\\n";
        case '\t': return "\\t";
        case '\r': return "\\r";
        case '\0': return "\\0";
        case '\\': return "\\\\";
        default:
            if (c >= 32 && c < 127) return std::string(1, (char)c);
            char buf[8];
            std::snprintf(buf, sizeof(buf), "\\x%02x", c);
            return buf;
    }
}

std::string charset_summary(const std::vector<bool>& cs, size_t max_chars) {
    std::string out;
    size_t count = 0;
    for (int i = 0; i < 256; ++i) {
        if (!cs[i]) continue;
        if (count < max_chars) out += escape_for_display((unsigned char)i);
        ++count;
    }
    if (count > max_chars) {
        out += "...(";
        out += std::to_string(count);
        out += ")";
    }
    return out;
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::istringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delim))
        parts.push_back(token);
    return parts;
}

std::string join(const std::vector<std::string>& parts, const std::string& sep) {
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) result += sep;
        result += parts[i];
    }
    return result;
}

std::string trim(std::string_view s) {
    size_t lo = 0, hi = s.size();
    while (lo < hi && is_space_char((unsigned char)s[lo])) ++lo;
    while (hi > lo && is_space_char((unsigned char)s[hi - 1])) --hi;
    return std::string(s.substr(lo, hi - lo));
}

std::string to_lower(std::string s) {
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

std::string to_upper(std::string s) {
    for (char& c : s) c = (char)std::toupper((unsigned char)c);
    return s;
}

std::string repeat_str(const std::string& s, size_t n) {
    std::string result;
    result.reserve(s.size() * n);
    for (size_t i = 0; i < n; ++i) result += s;
    return result;
}

bool starts_with(std::string_view s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

bool ends_with(std::string_view s, std::string_view suffix) {
    return s.size() >= suffix.size() &&
           s.substr(s.size() - suffix.size()) == suffix;
}

std::string replace_all(std::string s, std::string_view from, std::string_view to) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}

}  // namespace utils
}  // namespace bishop
