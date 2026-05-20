#ifndef LINK_H
#define LINK_H

#include <memory>

class Node;
class Packet;

class Link {
    std::weak_ptr<Node> endpointA;
    std::weak_ptr<Node> endpointB;

public:
    Link(std::shared_ptr<Node> a, std::shared_ptr<Node> b);

    void transfer(std::shared_ptr<Node> sender, const Packet& pkt);
    std::shared_ptr<Node> getOtherNode(std::shared_ptr<Node> me) const;
};

#endif