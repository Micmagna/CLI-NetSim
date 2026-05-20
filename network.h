#ifndef NETWORK_H
#define NETWORK_H

#include <map>
#include <vector>
#include <memory>
#include <string>
#include "node.h"

class Link;

class Network {
    std::map<std::string, std::shared_ptr<Node>> nodes;
    std::vector<std::shared_ptr<Link>> links;

public:
    void addNode(const std::string& name, const std::string& type = "host");
    void setNodeIP(const std::string& name, const std::string& ip_str);
    void connectNodes(const std::string& a, const std::string& b);
    void sendMessage(const std::string& src, const std::string& dst, const std::string& msg);
    void showTopology() const;

private:
    std::shared_ptr<Node> getNode(const std::string& name);
};

#endif