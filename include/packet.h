#ifndef PACKET_H
#define PACKET_H
#include "ipaddress.h"
#include <string>

struct Packet {
    IPAddress src;
    IPAddress dst;
    uint8_t protocol;   // 1=ICMP, 6=TCP, 17=UDP, 253=TEST 
    int ttl;

    // ICMP fields (meaningful if protocol == 1)
    uint8_t  icmp_type;
    uint8_t  icmp_code;
    uint16_t icmp_id;
    uint16_t icmp_seq;

    std::string payload;   

    // Test packet constructor (send command)
    Packet(const IPAddress& s, const IPAddress& d, const std::string& msg)
        : src(s), dst(d), protocol(253), ttl(64),
          icmp_type(0), icmp_code(0), icmp_id(0), icmp_seq(0),
          payload(msg) {}

    // ICMP packet constructor
    Packet(const IPAddress& s, const IPAddress& d,
           uint8_t itype, uint8_t icode, uint16_t iid, uint16_t iseq,
           int t = 64)
        : src(s), dst(d), protocol(1), ttl(t),
          icmp_type(itype), icmp_code(icode), icmp_id(iid), icmp_seq(iseq) {}

    // Generic constructor
    Packet() : protocol(253), ttl(64), icmp_type(0), icmp_code(0), icmp_id(0), icmp_seq(0) {}
};

#endif