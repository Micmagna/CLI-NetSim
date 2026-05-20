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
    IPAddress(uint32_t address, uint8_t pref);   // nuovo costruttore

    std::string toString() const;
    bool sameSubnet(const IPAddress& other) const;
    uint32_t getAddress() const { return addr; }
    uint8_t getPrefix() const { return prefix; }
    bool operator==(const IPAddress& other) const;
};

#endif