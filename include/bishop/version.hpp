#ifndef BISHOP_VERSION_HPP_
#define BISHOP_VERSION_HPP_

#include <string>

namespace bishop {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 2;
constexpr int VERSION_PATCH = 0;

constexpr const char* VERSION_STRING = "0.2.0";

constexpr int version_number() {
    return VERSION_MAJOR * 10000 + VERSION_MINOR * 100 + VERSION_PATCH;
}

inline std::string version_string() { return VERSION_STRING; }

}  // namespace bishop
#endif
