#include "routernode.h"
#include "link.h"
#include <iostream>
#include <algorithm>

RouterNode::RouterNode(const std::string& n) : Node(n) {}

void RouterNode::addRoute(const IPAddress& network, const IPAddress& nextHop,
                          std::shared_ptr<Link> link, int metric) {
    for (const auto& entry : routingTable) {
        if (entry.network == network && entry.nextHop == nextHop && entry.outLink == link) {
            return;
        }
    }
    routingTable.push_back({network, nextHop, link, metric});
}

bool RouterNode::findBestRoute(const IPAddress& dest, RouteEntry& outEntry) {
    int bestPrefix = -1;
    bool found = false;
    for (const auto& entry : routingTable) {
        uint32_t mask = (entry.network.getPrefix() == 0) ? 0 : (~0u << (32 - entry.network.getPrefix()));
        if ((dest.getAddress() & mask) == (entry.network.getAddress() & mask)) {
            if (static_cast<int>(entry.network.getPrefix()) > bestPrefix) {
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
            if (pkt.icmp_type == 8) {
                Packet reply(ip, pkt.src, 0, 0, pkt.icmp_id, pkt.icmp_seq);
                sendPacket(reply);
                std::cout << name << " (router) responds to ping from " << pkt.src.toString() << "\n";
            } else {
                std::cout << name << " (router) receives ICMP type " << (int)pkt.icmp_type << "\n";
            }
        } else {
            std::cout << name << " (router) receives packet protocol " << (int)pkt.protocol
                      << " for itself, discarded.\n";
        }
        return;
    }
    forwardPacket(pkt);
}

void RouterNode::printRoutingTable() const {
    std::cout << "Routing table of " << name << " (metrics in hops):\n";
    for (const auto& entry : routingTable) {
        std::string nextHopStr = (entry.nextHop.getAddress() == 0) ? "direct" : entry.nextHop.toString();
        std::cout << "  " << entry.network.toString() << " via " << nextHopStr
                  << " metric " << entry.metric;
        auto other = entry.outLink->getOtherNode(std::const_pointer_cast<Node>(shared_from_this()));
        if (other) std::cout << " (link to " << other->getName() << ")";
        std::cout << "\n";
    }
}

std::vector<RouteEntry> RouterNode::getRoutingTable() const {
    return routingTable;
}

bool RouterNode::receiveRoutingUpdate(const std::vector<RouteEntry>& senderTable,
                                      std::shared_ptr<Node> sender) {
    bool changed = false;
    for (const auto& entry : senderTable) {
        int newMetric = entry.metric + 1;
        IPAddress destNetwork = entry.network;

        // Search if we already have a route to this network
        auto it = std::find_if(routingTable.begin(), routingTable.end(),
            [&destNetwork](const RouteEntry& r) { return r.network == destNetwork; });

        if (it == routingTable.end()) {
            // New network: add it
            IPAddress nextHopIP = sender->getIP();
            if (nextHopIP.getAddress() == 0) continue;
            // Find the link to the sender
            std::shared_ptr<Link> linkToSender = nullptr;
            for (auto& lnk : links) {
                auto other = lnk->getOtherNode(shared_from_this());
                if (other == sender) {
                    linkToSender = lnk;
                    break;
                }
            }
            if (!linkToSender) continue;
            routingTable.push_back({destNetwork, nextHopIP, linkToSender, newMetric});
            changed = true;
            std::cout << name << " has learned network " << destNetwork.toString()
                      << " via " << nextHopIP.toString() << " (metric " << newMetric << ")\n";
        } else {
            // Network already known: update if necessary
            if (it->nextHop.getAddress() == sender->getIP().getAddress()) {
                if (it->metric != newMetric) {
                    it->metric = newMetric;
                    changed = true;
                }
            } else if (newMetric < it->metric) {
                // Better path through another neighbor
                std::shared_ptr<Link> linkToSender = nullptr;
                for (auto& lnk : links) {
                    auto other = lnk->getOtherNode(shared_from_this());
                    if (other == sender) {
                        linkToSender = lnk;
                        break;
                    }
                }
                if (linkToSender) {
                    it->nextHop = sender->getIP();
                    it->outLink = linkToSender;
                    it->metric = newMetric;
                    changed = true;
                    std::cout << name << " has updated network " << destNetwork.toString()
                              << " via " << sender->getIP().toString() << " (metric " << newMetric << ")\n";
                }
            }
        }
    }
    return changed;
}