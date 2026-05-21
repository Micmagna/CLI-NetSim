#ifndef ROUTERNODE_H
#define ROUTERNODE_H

#include "node.h"
#include <vector>

struct RouteEntry {
    IPAddress network;   // destinazione
    IPAddress nextHop;   // 0.0.0.0 per reti dirette
    std::shared_ptr<Link> outLink;
    int metric;          // numero di hop (1 = direttamente connessa)

    RouteEntry() : metric(1) {}
    RouteEntry(const IPAddress& net, const IPAddress& nh, std::shared_ptr<Link> link, int m = 1)
        : network(net), nextHop(nh), outLink(link), metric(m) {}
};

class RouterNode : public Node {
    std::vector<RouteEntry> routingTable;

    bool findBestRoute(const IPAddress& dest, RouteEntry& outEntry);
    void forwardPacket(const Packet& pkt);
    void sendICMPDestUnreachable(const Packet& original);

protected:
    void handleNonLocalPacket(const Packet& pkt) override;
    void onTTLExpired(const Packet& pkt) override;

public:
    RouterNode(const std::string& n);
    std::string getType() const override { return "router"; }
    void addRoute(const IPAddress& network, const IPAddress& nextHop,
                  std::shared_ptr<Link> link, int metric = 1);
    void processIncomingPacket(const Packet& pkt) override;
    void printRoutingTable() const;
    std::vector<RouteEntry> getRoutingTable() const;
    bool receiveRoutingUpdate(const std::vector<RouteEntry>& senderTable, 
                  std::shared_ptr<Node> sender, const IPAddress& senderIP);
    void buildDirectRoutes();
    bool isOwnIP(uint32_t addr) const;
};

#endif