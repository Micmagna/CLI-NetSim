#include "network.h"
#include "hostnode.h"
#include "routernode.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

void Network::addNode(const std::string& name, const std::string& type) {
    if (nodes.find(name) != nodes.end())
        throw std::runtime_error("Node '" + name + "' already exists.");
    std::shared_ptr<Node> node;
    if (type == "host") {
        node = std::make_shared<HostNode>(name);
    } else if (type == "router") {
        node = std::make_shared<RouterNode>(name);
    } else {
        throw std::runtime_error("Unknown node type: " + type);
    }
    nodes[name] = node;
    std::cout << "Node " << name << " of type " << type << " created.\n";
}

// New version with interface
void Network::setNodeIP(const std::string& node, const std::string& ifname, const std::string& ip_str) {
    auto n = getNode(node);
    n->addInterface(ifname);
    auto iface = n->getInterface(ifname);
    IPAddress ip(ip_str);
    // Check uniqueness across all interfaces of all nodes
    for (const auto& [otherName, otherNode] : nodes) {
        for (const auto& [otherIfname, otherIface] : otherNode->getInterfaces()) {
            if (otherIface->getIP().getAddress() == ip.getAddress()) {
                throw std::runtime_error("IP " + ip.toString() + " already in use by " + otherName + ":" + otherIfname);
            }
        }
    }
    iface->setIP(ip);
    std::cout << "Set IP " << ip.toString() << " on " << node << ":" << ifname << "\n";
}

// Old version (backward compatibility) – uses default interface "eth0"
void Network::setNodeIP(const std::string& name, const std::string& ip_str) {
    setNodeIP(name, "eth0", ip_str);
}

void Network::connectNodes(const std::string& a, const std::string& ifA,
                           const std::string& b, const std::string& ifB) {
    auto nodeA = getNode(a);
    auto nodeB = getNode(b);
    nodeA->addInterface(ifA);
    nodeB->addInterface(ifB);
    auto link = std::make_shared<Link>(nodeA, nodeB);
    links.push_back(link);
    nodeA->getInterface(ifA)->connect(link);
    nodeB->getInterface(ifB)->connect(link);
    std::cout << "Connected " << a << ":" << ifA << " to " << b << ":" << ifB << "\n";

    if (nodeA->getType() == "router") {
        auto r = std::static_pointer_cast<RouterNode>(nodeA);
        r->buildDirectRoutes();
    }
    if (nodeB->getType() == "router") {
        auto r = std::static_pointer_cast<RouterNode>(nodeB);
        r->buildDirectRoutes();
    }
}

// Old version (backward compatibility) – uses default interface "eth0" for both
void Network::connectNodes(const std::string& a, const std::string& b) {
    connectNodes(a, "eth0", b, "eth0");
}

void Network::sendMessage(const std::string& src, const std::string& dst, const std::string& msg) {
    auto srcNode = getNode(src);
    auto dstNode = getNode(dst);
    Packet pkt(srcNode->getFirstIP(), dstNode->getFirstIP(), msg);
    srcNode->sendPacket(pkt);
}

void Network::showTopology() const {
    std::cout << "\n--- Nodes ---\n";
    for (const auto& [n, nd] : nodes) {
        std::cout << n << " (" << nd->getType() << ") ";
        auto ip = nd->getFirstIP();
        std::cout << (ip.getAddress() ? ip.toString() : "no IP") << "\n";
    }
    std::cout << "--- Links ---\n";
    for (const auto& lnk : links) {
        std::string nameA = "?", nameB = "?";
        for (const auto& [n, nd] : nodes) {
            if (lnk->getOtherNode(nd) != nullptr) {
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

void Network::ping(const std::string& srcName, const std::string& dstName, int count) {
    auto srcNode = getNode(srcName);
    auto dstNode = getNode(dstName);
    uint16_t id = 0x1234;
    for (int seq = 1; seq <= count; ++seq) {
        Packet echoReq(srcNode->getFirstIP(), dstNode->getFirstIP(), 8, 0, id, (uint16_t)seq);
        std::cout << srcName << " sends ICMP Echo Request to " << dstName
                  << " (seq=" << seq << ")\n";
        srcNode->sendPacket(echoReq);
    }
}

void Network::traceroute(const std::string& srcName, const std::string& dstName) {
    auto srcNode = getNode(srcName);
    auto dstNode = getNode(dstName);
    const int MAX_TTL = 30;
    uint16_t id = 0xABCD;
    std::cout << "traceroute to " << dstNode->getFirstIP().toString() << " from " << srcName << "\n";
    for (int ttl = 1; ttl <= MAX_TTL; ++ttl) {
        srcNode->resetICMPEvent();
        Packet probe(srcNode->getFirstIP(), dstNode->getFirstIP(), 8, 0, id, (uint16_t)ttl, ttl);
        srcNode->sendPacket(probe);

        IPAddress eventSrc;
        uint8_t eventType;
        if (srcNode->getAndClearICMPEvent(eventSrc, eventType)) {
            if (eventType == 0) {
                std::cout << ttl << ": " << eventSrc.toString() << " (destination reached)\n";
                break;
            } else if (eventType == 11) {
                std::cout << ttl << ": " << eventSrc.toString() << "\n";
            } else if (eventType == 3) {
                std::cout << ttl << ": " << eventSrc.toString() << " ICMP Dest Unreachable\n";
                break;
            } else {
                std::cout << ttl << ": ICMP type " << (int)eventType << " from " << eventSrc.toString() << "\n";
            }
        } else {
            std::cout << ttl << ": *\n";
        }
    }
}

void Network::routeShow(const std::string& router) {
    auto r = std::dynamic_pointer_cast<RouterNode>(getNode(router));
    if (!r) throw std::runtime_error(router + " is not a router.");
    r->printRoutingTable();
}

void Network::routeUpdate() {
    // 1. Build direct routes for all routers based on their interfaces
    for (auto& [name, node] : nodes) {
        if (node->getType() == "router") {
            auto router = std::static_pointer_cast<RouterNode>(node);
            router->buildDirectRoutes();   // creates routes for each interface with an IP
        }
    }

    // 2. Distance Vector exchange until convergence (or max iterations)
    bool globalChanged = true;
    int iterations = 0;
    const int MAX_ITER = 50;
    while (globalChanged && iterations < MAX_ITER) {
        globalChanged = false;
        iterations++;
        for (auto& [rname, rnode] : nodes) {
            if (rnode->getType() != "router") continue;
            auto router = std::static_pointer_cast<RouterNode>(rnode);
            // Send updates to all neighboring routers via each connected interface
            for (auto& [ifname, iface] : router->getInterfaces()) {
                auto lnk = iface->getLink();
                if (!lnk) continue;
                auto neighbor = lnk->getOtherNode(router);
                if (!neighbor || neighbor->getType() != "router") continue;

                // Get the IP of the neighbor on the interface connected to this link
                auto neighIface = neighbor->getInterfaceByLink(lnk);
                if (!neighIface) continue;
                IPAddress neighIP = neighIface->getIP();

                auto neighRouter = std::static_pointer_cast<RouterNode>(neighbor);
                // Split horizon filtering
                std::vector<RouteEntry> filteredTable;
                for (const auto& entry : neighRouter->getRoutingTable()) {
                    if (!router->isOwnIP(entry.nextHop.getAddress())) {
                        filteredTable.push_back(entry);
                    }
                }
                bool changed = router->receiveRoutingUpdate(filteredTable, neighbor, neighIP);
                if (changed) globalChanged = true;
            }
        }
    }

    if (iterations >= MAX_ITER) {
        std::cout << "Warning: maximum number of iterations reached, possible loop.\n";
    } else {
        std::cout << "Route update completed (" << iterations << " iterations).\n";
    }
}

std::shared_ptr<Node> Network::getNode(const std::string& name) {
    auto it = nodes.find(name);
    if (it == nodes.end())
        throw std::runtime_error("Node '" + name + "' not found.");
    return it->second;
}

std::vector<std::string> Network::getNodeNames() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : nodes) names.push_back(name);
    return names;
}