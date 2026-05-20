#ifndef HOSTNODE_H
#define HOSTNODE_H
#include "node.h"

class HostNode : public Node {
    void handleICMP(const Packet& pkt);
public:
    HostNode(const std::string& n);
    std::string getType() const override;
    void processIncomingPacket(const Packet& pkt) override;
};

#endif