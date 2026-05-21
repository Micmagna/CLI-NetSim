#ifndef NODE_H
#define NODE_H

#include <memory>
#include <vector>
#include <map>
#include <string>
#include "ipaddress.h"
#include "packet.h"
#include "interface.h"   // NEW

class Link;  // already forward declared

class Node : public std::enable_shared_from_this<Node> {
protected:
    std::string name;
    std::map<std::string, std::shared_ptr<Interface>> interfaces;   // NEW
    std::vector<std::shared_ptr<Link>> links;   // keep for backward compatibility, may be removed later
    virtual void handleNonLocalPacket(const Packet& pkt);

    // ICMP events... (keep as before)
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

    void addInterface(const std::string& ifname);
    std::shared_ptr<Interface> getInterface(const std::string& ifname) const;
    const std::map<std::string, std::shared_ptr<Interface>>& getInterfaces() const { return interfaces; }
    IPAddress getFirstIP() const;
    std::shared_ptr<Interface> getInterfaceByLink(std::shared_ptr<Link> link) const;
    bool isDestinationLocal(const IPAddress& addr) const;
    void addLink(std::shared_ptr<Link> link);
    virtual void sendPacket(const Packet& pkt);
    void receivePacket(const Packet& pkt);
    virtual void processIncomingPacket(const Packet& pkt) = 0;
    virtual bool filterPacket(const Packet& pkt, bool outgoing) { return true; }
    virtual std::string getType() const = 0;

    // ICMP event accessors (used by traceroute)
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
    virtual void onTTLExpired(const Packet& pkt) { /* default: drop */ }
};

#endif