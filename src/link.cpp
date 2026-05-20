#include "link.h"
#include "node.h"
#include "packet.h"
#include <iostream>

Link::Link(std::shared_ptr<Node> a, std::shared_ptr<Node> b)
    : endpointA(a), endpointB(b) {}

void Link::transfer(std::shared_ptr<Node> sender, const Packet& pkt) {
    auto spA = endpointA.lock();
    auto spB = endpointB.lock();
    if (!spA || !spB) {
        std::cout << "Broken link: one of the nodes no longer exists.\n";
        return;
    }
    std::shared_ptr<Node> receiver = (spA == sender) ? spB : spA;
    receiver->receivePacket(pkt);
}

std::shared_ptr<Node> Link::getOtherNode(std::shared_ptr<Node> me) const {
    auto spA = endpointA.lock();
    auto spB = endpointB.lock();
    if (spA == me) return spB;
    if (spB == me) return spA;
    return nullptr;
}