#ifndef NODE_H
#define NODE_H

#include <memory>
#include <vector>
#include <string>
#include "ipaddress.h"
#include "packet.h"

class Link;  // forward declaration

class Node : public std::enable_shared_from_this<Node> {
protected:
    std::string name;
    IPAddress ip;
    std::vector<std::shared_ptr<Link>> links;

public:
    Node(const std::string& n);
    virtual ~Node() = default;

    const std::string& getName() const { return name; }
    IPAddress getIP() const { return ip; }
    void setIP(const IPAddress& new_ip);

    void addLink(std::shared_ptr<Link> link);

    virtual void sendPacket(const Packet& pkt);
    virtual void receivePacket(const Packet& pkt) = 0;
    virtual std::string getType() const = 0;
};

#endif