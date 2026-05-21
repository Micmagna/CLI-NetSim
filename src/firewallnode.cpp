#include "firewallnode.h"
#include "link.h"
#include <iostream>
#include <algorithm>

FirewallNode::FirewallNode(const std::string& n) : Node(n) {}

bool FirewallRule::matches(const Packet& pkt) const {
    // Check protocol (0 = any)
    if (protocol != 0 && pkt.protocol != protocol) return false;
    // Check source IP
    if (src.getAddress() != 0) {
        if (pkt.src.getAddress() != src.getAddress()) return false;
    }
    // Check destination IP
    if (dst.getAddress() != 0) {
        if (pkt.dst.getAddress() != dst.getAddress()) return false;
    }
    return true;
}

void FirewallNode::addRule(const FirewallRule& rule) {
    rules.push_back(rule);
}

void FirewallNode::removeRule(int index) {
    if (index >= 1 && index <= static_cast<int>(rules.size())) {
        rules.erase(rules.begin() + (index - 1));
    }
}

void FirewallNode::handleNonLocalPacket(const Packet& pkt) {
    // Determine incoming interface by matching the source IP of the packet
    std::shared_ptr<Link> incomingLink = nullptr;
    for (auto& [ifname, iface] : interfaces) {
        auto lnk = iface->getLink();
        if (!lnk) continue;
        auto other = lnk->getOtherNode(shared_from_this());
        if (!other) continue;
        for (auto& [otherIfname, otherIface] : other->getInterfaces()) {
            if (otherIface->getIP().getAddress() == pkt.src.getAddress()) {
                incomingLink = lnk;
                break;
            }
        }
        if (incomingLink) break;
    }

    if (!incomingLink) {
        std::cout << name << " (firewall): could not determine incoming interface, dropping packet.\n";
        return;
    }

    // Apply rules in order (first match wins)
    bool allowed = false;
    for (const auto& rule : rules) {
        if (rule.matches(pkt)) {
            allowed = (rule.action == FirewallRule::ALLOW);
            break;
        }
    }
    // If no rule matched, default is to allow (empty ruleset → pass all)
    if (rules.empty()) allowed = true;

    if (!allowed) {
        std::cout << name << " (firewall): packet blocked by rule.\n";
        return;
    }

    // Forward out the other interface (bridge mode, works best with exactly two interfaces)
    for (auto& [ifname, iface] : interfaces) {
        auto lnk = iface->getLink();
        if (lnk && lnk != incomingLink) {
            std::cout << name << " (firewall) forwards packet from " << pkt.src.toString()
                      << " to " << pkt.dst.toString() << "\n";
            lnk->transfer(shared_from_this(), pkt);
            return;
        }
    }
    std::cout << name << " (firewall): no outgoing interface found, dropping packet.\n";
}

void FirewallNode::processIncomingPacket(const Packet& pkt) {
    if (pkt.protocol == 1 && pkt.icmp_type == 8) {
        Packet reply(getFirstIP(), pkt.src, 0, 0, pkt.icmp_id, pkt.icmp_seq);
        sendPacket(reply);
        std::cout << name << " (firewall) responds to ping from " << pkt.src.toString() << "\n";
    } else {
        std::cout << name << " (firewall) received packet for itself, protocol "
                  << (int)pkt.protocol << ", dropped.\n";
    }
}