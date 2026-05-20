#include "node.h"
#include "link.h"  
#include <iostream>

Node::Node(const std::string& n) : name(n) {}

void Node::setIP(const IPAddress& new_ip) { ip = new_ip; }

void Node::addLink(std::shared_ptr<Link> link) {
    links.push_back(link);
}

void Node::sendPacket(const Packet& pkt) {
    for (auto& lnk : links) {
        auto other = lnk->getOtherNode(shared_from_this());
        if (other && other->getIP().getAddress() == pkt.dst.getAddress()) {
            lnk->transfer(shared_from_this(), pkt);
            return;
        }
    }
    std::cout << "Error: " << name << " is not directly connected to the recipient "
              << pkt.dst.toString() << "\n";
}