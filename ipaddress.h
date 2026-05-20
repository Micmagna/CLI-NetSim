#ifndef IPADDRESS_H
#define IPADDRESS_H

#include <string>
#include <cstdint>
#include <stdexcept>

class IPAddress {
    uint32_t addr;   // network byte order
    uint8_t  prefix;
public:
    IPAddress();
    explicit IPAddress(const std::string& cidr);

    std::string toString() const;
    bool sameSubnet(const IPAddress& other) const;
    uint32_t getAddress() const { return addr; }
    bool operator==(const IPAddress& other) const;
};

#endif