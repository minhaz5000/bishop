#include <cstdint>
#include <string>
#include "regex.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size == 0) return 0;
    std::string pattern(reinterpret_cast<const char*>(data), size);
    bishop::Regex re;
    re.compile(pattern);
    re.match("fuzztest");
    return 0;
}
