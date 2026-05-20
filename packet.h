#ifndef PACKET_H
#define PACKET_H

#include "ipaddress.h"
#include <string>

struct Packet {
    IPAddress src;
    IPAddress dst;
    std::string payload;
    int ttl;

    Packet(const IPAddress& s, const IPAddress& d, const std::string& msg)
        : src(s), dst(d), payload(msg), ttl(64) {}
};

#endif