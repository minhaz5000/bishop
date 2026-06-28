#include <cstdint>
#include <string>

#include "bishop/regex.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 2) return 0;
    size_t sep = 0;
    while (sep < size && data[sep] != '\n') ++sep;

    std::string pattern(reinterpret_cast<const char*>(data), sep);
    std::string text(reinterpret_cast<const char*>(data + (sep < size ? sep + 1 : sep)),
                     sep < size ? size - sep - 1 : 0);

    try {
        bishop::Regex re;
        re.compile(pattern);
        re.match(text);
        re.search(text);
    }
    catch (const bishop::RegexError&) {
        return 0;
    }
    return 0;
}
