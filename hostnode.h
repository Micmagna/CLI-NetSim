#ifndef HOSTNODE_H
#define HOSTNODE_H

#include "node.h"

class HostNode : public Node {
public:
    HostNode(const std::string& n);

    std::string getType() const override;
    void receivePacket(const Packet& pkt) override;
};

#endif