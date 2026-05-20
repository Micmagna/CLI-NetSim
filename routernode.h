#ifndef ROUTERNODE_H
#define ROUTERNODE_H

#include "node.h"
#include <vector>

struct RouteEntry {
    IPAddress network;   
    IPAddress nextHop;   
    std::shared_ptr<Link> outLink;
};

class RouterNode : public Node {
    std::vector<RouteEntry> routingTable;

    bool findBestRoute(const IPAddress& dest, RouteEntry& outEntry);
    void forwardPacket(const Packet& pkt);
    void sendICMPDestUnreachable(const Packet& original);

protected:
    void onTTLExpired(const Packet& pkt) override;

public:
    RouterNode(const std::string& n);
    std::string getType() const override { return "router"; }

    void addRoute(const IPAddress& network, const IPAddress& nextHop, std::shared_ptr<Link> link);
    void processIncomingPacket(const Packet& pkt) override;
    void printRoutingTable() const;
};

#endif