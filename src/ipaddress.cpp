#include "ipaddress.h"
#include <arpa/inet.h>
#include <sstream>

IPAddress::IPAddress() : addr(0), prefix(0) {}

IPAddress::IPAddress(const std::string& cidr) {
    size_t slash = cidr.find('/');
    if (slash == std::string::npos)
        throw std::invalid_argument("Formato IP errato: manca /prefisso. Usa X.X.X.X/P");

    std::string ip_str = cidr.substr(0, slash);
    std::string prefix_str = cidr.substr(slash + 1);

    struct in_addr inaddr;
    if (inet_pton(AF_INET, ip_str.c_str(), &inaddr) != 1)
        throw std::invalid_argument("IP non valido: " + ip_str);
    addr = inaddr.s_addr;

    int p = std::stoi(prefix_str);
    if (p < 0 || p > 32)
        throw std::invalid_argument("Prefisso non valido: " + prefix_str);
    prefix = static_cast<uint8_t>(p);
}

IPAddress::IPAddress(uint32_t address, uint8_t pref) : addr(address), prefix(pref) {}

std::string IPAddress::toString() const {
    char buf[INET_ADDRSTRLEN];
    struct in_addr in{ addr };
    inet_ntop(AF_INET, &in, buf, sizeof(buf));
    return std::string(buf) + "/" + std::to_string(prefix);
}

bool IPAddress::sameSubnet(const IPAddress& other) const {
    uint32_t mask = (prefix == 0) ? 0 : (~0u << (32 - prefix));
    return (addr & mask) == (other.addr & mask);
}

bool IPAddress::operator==(const IPAddress& other) const {
    return addr == other.addr && prefix == other.prefix;
}