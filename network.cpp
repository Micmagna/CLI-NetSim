#include "network.h"
#include "hostnode.h"
#include "link.h"
#include <iostream>
#include <stdexcept>
#include <sstream>

void Network::addNode(const std::string& name, const std::string& type) {
    if (nodes.find(name) != nodes.end())
        throw std::runtime_error("Node '" + name + "' already existing.");
    
    std::shared_ptr<Node> node;
    if (type == "host") {
        node = std::make_shared<HostNode>(name);
    } else {
        throw std::runtime_error("Unknown node type: " + type);
    }
    nodes[name] = node;
    std::cout << "Node " << name << " of type " << type << " created.\n";
}

void Network::setNodeIP(const std::string& name, const std::string& ip_str) {
    auto it = nodes.find(name);
    if (it == nodes.end())
        throw std::runtime_error("Node '" + name + "' does not exist.");
    IPAddress ip(ip_str);
    // verifica unicità indirizzo
    for (const auto& [n, nd] : nodes) {
        if (nd->getIP().getAddress() == ip.getAddress() && n != name)
            throw std::runtime_error("IP " + ip.toString() + " already in use by " + n);
    }
    it->second->setIP(ip);
    std::cout << "IP " << ip.toString() << " set on " << name << ".\n";
}

void Network::connectNodes(const std::string& a, const std::string& b) {
    auto nodeA = getNode(a);
    auto nodeB = getNode(b);
    // controllo collegamenti duplicati
    for (auto& lnk : links) {
        auto other = lnk->getOtherNode(nodeA);
        if (other == nodeB)
            throw std::runtime_error(a + " and " + b + " are already connected.");
    }
    auto link = std::make_shared<Link>(nodeA, nodeB);
    links.push_back(link);
    nodeA->addLink(link);
    nodeB->addLink(link);
    std::cout << "Link created between " << a << " and " << b << ".\n";
}

void Network::sendMessage(const std::string& src, const std::string& dst, const std::string& msg) {
    auto srcNode = getNode(src);
    auto dstNode = getNode(dst);
    Packet pkt(srcNode->getIP(), dstNode->getIP(), msg);
    srcNode->sendPacket(pkt);
}

void Network::showTopology() const {
    std::cout << "\n--- Nodes ---\n";
    for (const auto& [n, nd] : nodes) {
        std::cout << n << " (" << nd->getType() << ") "
                  << (nd->getIP().toString().empty() ? "without IP" : nd->getIP().toString())
                  << "\n";
    }
    std::cout << "--- Links ---\n";
    for (const auto& lnk : links) {
        // For showing links, we get the names by iterating over the nodes
        // (simple solution but not efficient, good for debugging)
        std::string nameA = "?";
        std::string nameB = "?";
        for (const auto& [n, nd] : nodes) {
            if (lnk->getOtherNode(nd) != nullptr) {
                // This node participates in the link: the other endpoint is another node
                auto other = lnk->getOtherNode(nd);
                if (other) {
                    nameA = nd->getName();
                    nameB = other->getName();
                    break;
                }
            }
        }
        std::cout << nameA << " <--> " << nameB << "\n";
    }
    std::cout << "End of topology.\n";
}

std::shared_ptr<Node> Network::getNode(const std::string& name) {
    auto it = nodes.find(name);
    if (it == nodes.end())
        throw std::runtime_error("Node '" + name + "' does not exist.");
    return it->second;
}