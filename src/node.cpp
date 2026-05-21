#include "node.h"
#include "link.h"
#include <iostream>
#include <algorithm>

Node::Node(const std::string& n) : name(n) {}

void Node::addInterface(const std::string& ifname) {
    if (interfaces.find(ifname) == interfaces.end()) {
        interfaces[ifname] = std::make_shared<Interface>(ifname);
    }
}

std::shared_ptr<Interface> Node::getInterface(const std::string& ifname) const {
    auto it = interfaces.find(ifname);
    if (it == interfaces.end()) return nullptr;
    return it->second;
}

IPAddress Node::getFirstIP() const {
    if (!interfaces.empty()) {
        for (const auto& [name, iface] : interfaces) {
            if (iface->hasIP()) return iface->getIP();
        }
    }
    return IPAddress("0.0.0.0/0");
}

bool Node::isDestinationLocal(const IPAddress& addr) const {
    for (const auto& [ifname, iface] : interfaces) {
        if (iface->getIP().getAddress() == addr.getAddress()) return true;
    }
    return false;
}

void Node::sendPacket(const Packet& pkt) {
    if (!filterPacket(pkt, true)) {
        std::cout << name << " (firewall): outgoing packet blocked.\n";
        return;
    }
    // 1. Direct delivery: check if any neighbor's interface has the exact destination IP
    for (auto& [ifname, iface] : interfaces) {
        auto lnk = iface->getLink();
        if (!lnk) continue;
        auto other = lnk->getOtherNode(shared_from_this());
        if (!other) continue;
        // check all interfaces of other node
        for (auto& [other_ifname, other_iface] : other->getInterfaces()) {
            if (other_iface->getIP().getAddress() == pkt.dst.getAddress()) {
                lnk->transfer(shared_from_this(), pkt);
                return;
            }
        }
    }
    // 2. Forward to a router (using first available router neighbor)
    for (auto& [ifname, iface] : interfaces) {
        auto lnk = iface->getLink();
        if (!lnk) continue;
        auto other = lnk->getOtherNode(shared_from_this());
        if (other && other->getType() == "router") {
            std::cout << name << " forwards packet to router " << other->getName() << "\n";
            lnk->transfer(shared_from_this(), pkt);
            return;
        }
    }
    std::cout << "Error: " << name << " has no route to " << pkt.dst.toString() << "\n";
}

void Node::receivePacket(const Packet& pkt) {
    if (!filterPacket(pkt, false)) {
        std::cout << name << " (firewall): incoming packet blocked.\n";
        return;
    }
    Packet p = pkt;
    p.ttl--;
    if (p.ttl <= 0) {
        // TTL expired
        if (!isDestinationLocal(pkt.dst)) {
            onTTLExpired(pkt);   // let subclass send ICMP Time Exceeded if router
        } else {
            processIncomingPacket(p);   // even with TTL=0, deliver if for us
        }
        return;
    }
    // TTL > 0
    if (isDestinationLocal(p.dst)) {
        processIncomingPacket(p);
    } else {
        handleNonLocalPacket(p);   // forward if router, drop if host
    }
}


std::shared_ptr<Interface> Node::getInterfaceByLink(std::shared_ptr<Link> link) const {
    for (const auto& [ifname, iface] : interfaces) {
        if (iface->getLink() == link) return iface;
    }
    return nullptr;
}

void Node::handleNonLocalPacket(const Packet& pkt) {
    // Default: host discards non‑local packets
}