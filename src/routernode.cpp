#include "routernode.h"
#include "link.h"
#include <iostream>
#include <algorithm>

RouterNode::RouterNode(const std::string& n) : Node(n) {}

void RouterNode::addRoute(const IPAddress& network, const IPAddress& nextHop, std::shared_ptr<Link> link) {
    routingTable.push_back({network, nextHop, link});
}

bool RouterNode::findBestRoute(const IPAddress& dest, RouteEntry& outEntry) {
    int bestPrefix = -1;
    bool found = false;
    for (const auto& entry : routingTable) {
        uint32_t mask = (entry.network.getPrefix() == 0) ? 0 : (~0u << (32 - entry.network.getPrefix()));
        if ((dest.getAddress() & mask) == (entry.network.getAddress() & mask)) {
            if (entry.network.getPrefix() > bestPrefix) {
                bestPrefix = entry.network.getPrefix();
                outEntry = entry;
                found = true;
            }
        }
    }
    return found;
}

void RouterNode::forwardPacket(const Packet& pkt) {
    RouteEntry entry;
    if (findBestRoute(pkt.dst, entry)) {
        entry.outLink->transfer(shared_from_this(), pkt);
    } else {
        sendICMPDestUnreachable(pkt);
    }
}

void RouterNode::sendICMPDestUnreachable(const Packet& original) {
    Packet unreach(ip, original.src, 3, 0, 0, 0);
    sendPacket(unreach);
    std::cout << name << " (router): no route for " << original.dst.toString()
              << ", sending ICMP Dest Unreachable to " << original.src.toString() << "\n";
}

void RouterNode::onTTLExpired(const Packet& pkt) {
    Packet timeEx(ip, pkt.src, 11, 0, 0, 0);
    sendPacket(timeEx);
    std::cout << name << " (router): TTL expired, sending ICMP Time Exceeded to "
              << pkt.src.toString() << "\n";
}

void RouterNode::processIncomingPacket(const Packet& pkt) {
    if (pkt.dst.getAddress() == ip.getAddress()) {
        if (pkt.protocol == 1) {
            if (pkt.icmp_type == 8) { // Echo Request
                Packet reply(ip, pkt.src, 0, 0, pkt.icmp_id, pkt.icmp_seq);
                sendPacket(reply);
                std::cout << name << " (router) responds to ping from " << pkt.src.toString() << "\n";
            } else {
                std::cout << name << " (router) receives ICMP type " << (int)pkt.icmp_type << "\n";
            }
        } else {
            std::cout << name << " (router) receives packet with protocol " << (int)pkt.protocol
                      << " for itself, discarded.\n";
        }
        return;
    }
    forwardPacket(pkt);
}

void RouterNode::printRoutingTable() const {
    std::cout << "Routing Table of " << name << ":\n";
    for (const auto& entry : routingTable) {
        std::string nextHopStr = (entry.nextHop.getAddress() == 0) ? "direct" : entry.nextHop.toString();
        std::cout << "  " << entry.network.toString() << " via " << nextHopStr;
        auto other = entry.outLink->getOtherNode(std::const_pointer_cast<Node>(shared_from_this()));
        if (other) std::cout << " (link to " << other->getName() << ")";
        std::cout << "\n";
    }
}