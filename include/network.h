#ifndef NETWORK_H
#define NETWORK_H

#include <map>
#include <vector>
#include <memory>
#include <string>
#include "node.h"
#include "link.h"

class RouterNode;

class Network {
    std::map<std::string, std::shared_ptr<Node>> nodes;
    std::vector<std::shared_ptr<Link>> links;
    void autoAddDirectRoute(std::shared_ptr<Node> router, std::shared_ptr<Node> neighbor,
                            std::shared_ptr<Link> link);

public:
    void addNode(const std::string& name, const std::string& type = "host");
    std::shared_ptr<Node> getNode(const std::string& name);
    void sendMessage(const std::string& src, const std::string& dst, const std::string& msg);
    void showTopology() const;
    void ping(const std::string& src, const std::string& dst, int count = 4);
    void traceroute(const std::string& src, const std::string& dst);
    void routeShow(const std::string& router);
    void routeUpdate();   
    std::vector<std::string> getNodeNames() const;
    void setNodeIP(const std::string& node, const std::string& ifname, const std::string& ip_str);
    void connectNodes(const std::string& nodeA, const std::string& ifA,
                    const std::string& nodeB, const std::string& ifB);
};

#endif