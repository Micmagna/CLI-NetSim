#ifndef NODE_H
#define NODE_H

#include <memory>
#include <vector>
#include <string>
#include "ipaddress.h"
#include "packet.h"

class Link;

class Node : public std::enable_shared_from_this<Node> {
protected:
    std::string name;
    IPAddress ip;
    std::vector<std::shared_ptr<Link>> links;


    mutable IPAddress lastICMPSrc;
    mutable uint8_t lastICMPType = 0;
    mutable bool hasNewICMPEvent = false;

    void setICMPEvent(const IPAddress& src, uint8_t type) {
        lastICMPSrc = src;
        lastICMPType = type;
        hasNewICMPEvent = true;
    }

public:
    Node(const std::string& n);
    virtual ~Node() = default;

    const std::string& getName() const { return name; }
    IPAddress getIP() const { return ip; }
    void setIP(const IPAddress& new_ip);

    void addLink(std::shared_ptr<Link> link);
    const std::vector<std::shared_ptr<Link>>& getLinks() const { return links; }

    virtual void sendPacket(const Packet& pkt);
    void receivePacket(const Packet& pkt);   

    virtual void processIncomingPacket(const Packet& pkt) = 0;
    virtual bool filterPacket(const Packet& pkt, bool outgoing) { return true; }
    virtual std::string getType() const = 0;

    // Eventi ICMP per traceroute
    void resetICMPEvent() { hasNewICMPEvent = false; }
    bool getAndClearICMPEvent(IPAddress& src, uint8_t& type) {
        if (hasNewICMPEvent) {
            src = lastICMPSrc;
            type = lastICMPType;
            hasNewICMPEvent = false;
            return true;
        }
        return false;
    }

protected:
    virtual void onTTLExpired(const Packet& pkt) { /*Default: discard*/ }
};

#endif