#ifndef BISHOP_UTILS_HPP_
#define BISHOP_UTILS_HPP_

#include <string>
#include <string_view>
#include <vector>

namespace bishop {
namespace utils {

bool is_word_char(unsigned char c);
bool is_digit_char(unsigned char c);
bool is_space_char(unsigned char c);

std::string escape_for_display(unsigned char c);
std::string charset_summary(const std::vector<bool>& cs, size_t max_chars = 8);

std::vector<std::string> split(const std::string& s, char delim);
std::string join(const std::vector<std::string>& parts, const std::string& sep);
std::string trim(std::string_view s);
std::string to_lower(std::string s);
std::string to_upper(std::string s);
std::string repeat_str(const std::string& s, size_t n);
bool starts_with(std::string_view s, std::string_view prefix);
bool ends_with(std::string_view s, std::string_view suffix);
std::string replace_all(std::string s, std::string_view from, std::string_view to);

}  // namespace utils
}  // namespace bishop
#endif
