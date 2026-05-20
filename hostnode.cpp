#include "hostnode.h"
#include <iostream>

HostNode::HostNode(const std::string& n) : Node(n) {}

std::string HostNode::getType() const { return "host"; }

void HostNode::receivePacket(const Packet& pkt) {
    if (pkt.dst.getAddress() == ip.getAddress()) {
        std::cout << name << " receives from " << pkt.src.toString()
                  << ": " << pkt.payload << "\n";
    } else {
        std::cout << name << " discards packet ("
                  << pkt.dst.toString() << ")\n";
    }
}