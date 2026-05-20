#include "network.h"
#include "hostnode.h"
#include "routernode.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

void Network::addNode(const std::string& name, const std::string& type) {
    if (nodes.find(name) != nodes.end())
        throw std::runtime_error("Nodo '" + name + "' già esistente.");
    std::shared_ptr<Node> node;
    if (type == "host") {
        node = std::make_shared<HostNode>(name);
    } else if (type == "router") {
        node = std::make_shared<RouterNode>(name);
    } else {
        throw std::runtime_error("Tipo di nodo sconosciuto: " + type);
    }
    nodes[name] = node;
    std::cout << "Nodo " << name << " di tipo " << type << " creato.\n";
}

void Network::setNodeIP(const std::string& name, const std::string& ip_str) {
    auto it = nodes.find(name);
    if (it == nodes.end())
        throw std::runtime_error("Nodo '" + name + "' inesistente.");
    IPAddress ip(ip_str);
    for (const auto& [n, nd] : nodes) {
        if (nd->getIP().getAddress() == ip.getAddress() && n != name)
            throw std::runtime_error("IP " + ip.toString() + " già in uso da " + n);
    }
    it->second->setIP(ip);
    std::cout << "Impostato IP " << ip.toString() << " su " << name << ".\n";
}

void Network::autoAddDirectRoute(std::shared_ptr<Node> router, std::shared_ptr<Node> neighbor,
                                 std::shared_ptr<Link> link) {
    if (router->getType() != "router") return;
    auto r = std::static_pointer_cast<RouterNode>(router);
    IPAddress neighborIP = neighbor->getIP();
    if (neighborIP.getAddress() == 0) return;
    uint32_t mask = (neighborIP.getPrefix() == 0) ? 0 : (~0u << (32 - neighborIP.getPrefix()));
    uint32_t netAddr = neighborIP.getAddress() & mask;
    IPAddress network(netAddr, neighborIP.getPrefix());
    IPAddress zero("0.0.0.0/0");
    r->addRoute(network, zero, link);
    std::cout << router->getName() << " aggiunge rotta diretta: " << network.toString()
              << " via link to " << neighbor->getName() << "\n";
}

void Network::connectNodes(const std::string& a, const std::string& b) {
    auto nodeA = getNode(a);
    auto nodeB = getNode(b);
    for (auto& lnk : links) {
        auto other = lnk->getOtherNode(nodeA);
        if (other == nodeB)
            throw std::runtime_error(a + " e " + b + " sono già collegati.");
    }
    auto link = std::make_shared<Link>(nodeA, nodeB);
    links.push_back(link);
    nodeA->addLink(link);
    nodeB->addLink(link);
    std::cout << "Collegamento creato tra " << a << " e " << b << ".\n";

    // Auto routing per router
    autoAddDirectRoute(nodeA, nodeB, link);
    autoAddDirectRoute(nodeB, nodeA, link);
}

void Network::sendMessage(const std::string& src, const std::string& dst, const std::string& msg) {
    auto srcNode = getNode(src);
    auto dstNode = getNode(dst);
    Packet pkt(srcNode->getIP(), dstNode->getIP(), msg);
    srcNode->sendPacket(pkt);
}

void Network::showTopology() const {
    std::cout << "\n--- Nodi ---\n";
    for (const auto& [n, nd] : nodes) {
        std::cout << n << " (" << nd->getType() << ") "
                  << (nd->getIP().getAddress() ? nd->getIP().toString() : "senza IP")
                  << "\n";
    }
    std::cout << "--- Collegamenti ---\n";
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
    std::cout << "Fine topologia.\n";
}

void Network::ping(const std::string& srcName, const std::string& dstName, int count) {
    auto srcNode = getNode(srcName);
    auto dstNode = getNode(dstName);
    uint16_t id = 0x1234;
    for (int seq = 1; seq <= count; ++seq) {
        Packet echoReq(srcNode->getIP(), dstNode->getIP(), 8, 0, id, (uint16_t)seq);
        std::cout << srcName << " invia ICMP Echo Request a " << dstName
                  << " (seq=" << seq << ")\n";
        srcNode->sendPacket(echoReq);
    }
}

void Network::traceroute(const std::string& srcName, const std::string& dstName) {
    auto srcNode = getNode(srcName);
    auto dstNode = getNode(dstName);
    const int MAX_TTL = 30;
    uint16_t id = 0xABCD;
    std::cout << "traceroute to " << dstNode->getIP().toString() << " da " << srcName << "\n";
    for (int ttl = 1; ttl <= MAX_TTL; ++ttl) {
        srcNode->resetICMPEvent();
        Packet probe(srcNode->getIP(), dstNode->getIP(), 8, 0, id, (uint16_t)ttl, ttl);
        srcNode->sendPacket(probe);

        IPAddress eventSrc;
        uint8_t eventType;
        if (srcNode->getAndClearICMPEvent(eventSrc, eventType)) {
            if (eventType == 0) { // Echo Reply
                std::cout << ttl << ": " << eventSrc.toString() << " (destinazione raggiunta)\n";
                break;
            } else if (eventType == 11) { // Time Exceeded
                std::cout << ttl << ": " << eventSrc.toString() << "\n";
            } else if (eventType == 3) { // Dest Unreachable
                std::cout << ttl << ": " << eventSrc.toString() << " ICMP Dest Unreachable\n";
                break;
            } else {
                std::cout << ttl << ": ICMP tipo " << (int)eventType << " da " << eventSrc.toString() << "\n";
            }
        } else {
            std::cout << ttl << ": *\n";
        }
    }
}

void Network::routeAdd(const std::string& router, const std::string& networkStr,
                       const std::string& nextHopStr) {
    auto r = std::dynamic_pointer_cast<RouterNode>(getNode(router));
    if (!r) throw std::runtime_error(router + " non è un router.");
    IPAddress network(networkStr);
    IPAddress nextHop;
    bool isDirect = (nextHopStr == "direct" || nextHopStr == "0.0.0.0");
    if (isDirect) {
        nextHop = IPAddress("0.0.0.0/0");
    } else {
        nextHop = IPAddress(nextHopStr + "/32");
    }

    std::shared_ptr<Link> link = nullptr;
    if (isDirect) {
        // Cerca un vicino che appartenga alla stessa subnet
        for (auto& lnk : r->getLinks()) {
            auto other = lnk->getOtherNode(r);
            if (other && other->getIP().getAddress() != 0 && other->getIP().sameSubnet(network)) {
                link = lnk;
                break;
            }
        }
        if (!link) throw std::runtime_error("Nessun vicino diretto trovato per la rete " + networkStr);
    } else {
        // Cerca vicino con IP nextHop
        for (auto& lnk : r->getLinks()) {
            auto other = lnk->getOtherNode(r);
            if (other && other->getIP().getAddress() == nextHop.getAddress()) {
                link = lnk;
                break;
            }
        }
        if (!link) throw std::runtime_error("Next hop " + nextHopStr + " non è un vicino diretto di " + router);
    }

    r->addRoute(network, nextHop, link);
    std::cout << "Rotta aggiunta a " << router << ": " << network.toString()
              << " via " << (isDirect ? "direct" : nextHopStr) << "\n";
}

void Network::routeShow(const std::string& router) {
    auto r = std::dynamic_pointer_cast<RouterNode>(getNode(router));
    if (!r) throw std::runtime_error(router + " non è un router.");
    r->printRoutingTable();
}

std::shared_ptr<Node> Network::getNode(const std::string& name) {
    auto it = nodes.find(name);
    if (it == nodes.end())
        throw std::runtime_error("Nodo '" + name + "' inesistente.");
    return it->second;
}

std::vector<std::string> Network::getNodeNames() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : nodes) {
        names.push_back(name);
    }
    return names;
}