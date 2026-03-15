#include "bishop/serializer.hpp"

#include <cstring>
#include <vector>

namespace bishop {
namespace serial {

static void write_u8(std::ostream& o, uint8_t v)   { o.write((char*)&v, 1); }
static void write_u16(std::ostream& o, uint16_t v)  { o.write((char*)&v, 2); }
static void write_u32(std::ostream& o, uint32_t v)  { o.write((char*)&v, 4); }
static void write_i32(std::ostream& o, int32_t v)   { o.write((char*)&v, 4); }

static uint8_t  read_u8(std::istream& i)  { uint8_t v;  i.read((char*)&v, 1); return v; }
static uint16_t read_u16(std::istream& i) { uint16_t v; i.read((char*)&v, 2); return v; }
static uint32_t read_u32(std::istream& i) { uint32_t v; i.read((char*)&v, 4); return v; }
static int32_t  read_i32(std::istream& i) { int32_t v;  i.read((char*)&v, 4); return v; }

void serialize_dfa(const dfa::DFA& d, std::ostream& out)
{
    write_u32(out, MAGIC);
    write_u16(out, VERSION);
    write_u32(out, static_cast<uint32_t>(d.states.size()));
    write_i32(out, d.start);
    write_u8(out,  d.valid ? 1 : 0);

    for (const auto& s : d.states) {
        write_i32(out, s.id);
        write_u8(out,  s.is_accept ? 1 : 0);
        write_u32(out, static_cast<uint32_t>(s.transitions.size()));
        for (auto& [ch, dst] : s.transitions) {
            write_u8(out, static_cast<uint8_t>(ch));
            write_i32(out, dst);
        }
    }
}

dfa::DFA deserialize_dfa(std::istream& in)
{
    uint32_t magic = read_u32(in);
    if (magic != MAGIC)
        throw RegexError("serializer: bad magic number");

    uint16_t ver = read_u16(in);
    if (ver != VERSION)
        throw RegexError("serializer: unsupported version " +
                         std::to_string(ver));

    uint32_t state_count = read_u32(in);
    int32_t  start       = read_i32(in);
    uint8_t  valid_flag  = read_u8(in);

    dfa::DFA d;
    d.start = start;
    d.valid = valid_flag != 0;
    d.states.resize(state_count);

    for (uint32_t i = 0; i < state_count; ++i) {
        d.states[i].id        = read_i32(in);
        d.states[i].is_accept = read_u8(in) != 0;
        uint32_t tc           = read_u32(in);
        for (uint32_t j = 0; j < tc; ++j) {
            uint8_t ch  = read_u8(in);
            int32_t dst = read_i32(in);
            d.states[i].transitions[ch] = dst;
        }
    }

    if (!in)
        throw RegexError("serializer: truncated input");

    return d;
}

