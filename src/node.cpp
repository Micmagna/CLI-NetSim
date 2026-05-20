#include "node.h"
#include "link.h"
#include <iostream>

Node::Node(const std::string& n) : name(n) {}

void Node::setIP(const IPAddress& new_ip) { ip = new_ip; }
void Node::addLink(std::shared_ptr<Link> link) { links.push_back(link); }

void Node::sendPacket(const Packet& pkt) {
    if (!filterPacket(pkt, true)) {
        std::cout << name << " (firewall): outgoing packet blocked.\n";
        return;
    }

    // 1. Direct delivery: check if any neighbor has the exact destination IP
    for (auto& lnk : links) {
        auto other = lnk->getOtherNode(shared_from_this());
        if (other && other->getIP().getAddress() == pkt.dst.getAddress()) {
            lnk->transfer(shared_from_this(), pkt);
            return;
        }
    }

    // 2. Forward to a router if available (like a default gateway)
    for (auto& lnk : links) {
        auto other = lnk->getOtherNode(shared_from_this());
        if (other && other->getType() == "router") {
            std::cout << name << " forwards packet to router " << other->getName() << "\n";
            lnk->transfer(shared_from_this(), pkt);
            return;
        }
    }

    // 3. No route found
    std::cout << "Error: " << name << " has no direct link to " << pkt.dst.toString()
              << " and no router to forward the packet.\n";
}

void Node::receivePacket(const Packet& pkt) {
    if (!filterPacket(pkt, false)) {
        std::cout << name << " (firewall): incoming packet blocked.\n";
        return;
    }
    Packet p = pkt;
    p.ttl--;
    if (p.ttl <= 0) {
        if (p.dst.getAddress() != ip.getAddress()) {
            onTTLExpired(pkt);   // not for this node, but TTL expired: default is to drop, routers will override
        } else {
            processIncomingPacket(p);   // for this node, even if TTL expired, we still process it (e.g. for traceroute)
        }
        return;
    }
    if (p.dst.getAddress() == ip.getAddress()) {
        processIncomingPacket(p);
    } else {
        // not for this node, TTL>0: host discard, router overwrite this behavior to forward
    }
}