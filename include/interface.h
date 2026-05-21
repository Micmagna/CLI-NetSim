#ifndef INTERFACE_H
#define INTERFACE_H

#include <memory>
#include <string>
#include "ipaddress.h"

class Link;  // forward declaration

class Interface {
    std::string name;
    IPAddress ip;                        // 0.0.0.0/0 if not set
    std::weak_ptr<Link> connectedLink;   // weak to avoid cyclic refs

public:
    Interface(const std::string& n) : name(n) {}

    const std::string& getName() const { return name; }
    IPAddress getIP() const { return ip; }
    void setIP(const IPAddress& new_ip) { ip = new_ip; }
    bool hasIP() const { return ip.getAddress() != 0; }

    void connect(std::shared_ptr<Link> link);
    std::shared_ptr<Link> getLink() const { return connectedLink.lock(); }
};

#endif