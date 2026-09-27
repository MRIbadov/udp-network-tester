#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>

struct Packet {
    uint32_t sequenceNumber;
    uint64_t timestamp;
    char payload[1000];
};

#endif
