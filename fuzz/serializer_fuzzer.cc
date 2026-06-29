#include <cstdint>
#include <sstream>
#include <string>

#include "bishop/dfa.hpp"
#include "bishop/error.hpp"
#include "bishop/serializer.hpp"

static const std::string kProbes[] = {"",   "a", "abc", "\x00", "\xff\xfe\xfd", std::string(256, 'a'), "hello world",
                                      "123"};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 2) return 0;

    const bool use_text = (data[0] & 0x01) != 0;
    const std::string blob(reinterpret_cast<const char*>(data + 1), size - 1);

    try {
        std::istringstream iss(blob);
        dfa::DFA d;

        if (use_text) {
            d = bishop::serial::deserialize_dfa_text(iss);
        } else {
            d = bishop::serial::deserialize_dfa(iss);
        }

        if (d.valid && !d.states.empty()) {
            for (const auto& probe : kProbes) (void)dfa::dfa_match(d, probe);
        }
    }
    catch (const bishop::RegexError&) {
    }
    catch (const std::exception&) {
    }

    return 0;
}
