#ifndef BISHOP_SERIALIZER_HPP_
#define BISHOP_SERIALIZER_HPP_

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

#include "bishop/dfa.hpp"
#include "bishop/error.hpp"

namespace bishop {
namespace serial {

static constexpr uint32_t MAGIC   = 0x42495346u;
static constexpr uint16_t VERSION = 0x0001u;

struct Header {
    uint32_t magic;
    uint16_t version;
    uint32_t state_count;
    int32_t  start_state;
    uint8_t  flags;
};

void serialize_dfa(const dfa::DFA& d, std::ostream& out);
dfa::DFA deserialize_dfa(std::istream& in);

void serialize_dfa_text(const dfa::DFA& d, std::ostream& out);
dfa::DFA deserialize_dfa_text(std::istream& in);

}  // namespace serial
}  // namespace bishop
#endif
