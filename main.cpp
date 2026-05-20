#include <iostream>
#include <sstream>
#include <string>
#include "network.h"

int main() {
    Network net;
    std::string line;

    std::cout << "Network Simulator (C++ Packet Tracer light)\n";
    std::cout << "Commands:\n"
              << "  add node <name> [host|router]\n"
              << "  set ip <name> <IP/CIDR>\n"
              << "  connect <A> <B>\n"
              << "  send <A> <B> <message>\n"
              << "  ping <A> <B> [count]\n"
              << "  traceroute <A> <B>\n"
              << "  route add <router> <network> <next_hop|direct>\n"
              << "  route show <router>\n"
              << "  show\n"
              << "  exit\n";

    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        try {
            if (cmd == "add") {
                std::string type, name;
                iss >> type >> name;
                if (type != "node") {
                    std::cout << "Usage: add node <name> [host|router]\n";
                    continue;
                }
                std::string nodeType = "host";
                if (!iss.eof()) iss >> nodeType;
                net.addNode(name, nodeType);
            }
            else if (cmd == "set") {
                std::string what, name, ip;
                iss >> what >> name >> ip;
                if (what != "ip") {
                    std::cout << "Usage: set ip <name> <IP/CIDR>\n";
                    continue;
                }
                net.setNodeIP(name, ip);
            }
            else if (cmd == "connect") {
                std::string a, b;
                iss >> a >> b;
                net.connectNodes(a, b);
            }
            else if (cmd == "send") {
                std::string src, dst, msg;
                iss >> src >> dst;
                std::getline(iss, msg);
                if (!msg.empty() && msg[0] == ' ') msg.erase(0,1);
                net.sendMessage(src, dst, msg);
            }
            else if (cmd == "ping") {
                std::string src, dst;
                int count = 4;
                iss >> src >> dst;
                if (!iss.eof()) iss >> count;
                net.ping(src, dst, count);
            }
            else if (cmd == "traceroute") {
                std::string src, dst;
                iss >> src >> dst;
                net.traceroute(src, dst);
            }
            else if (cmd == "route") {
                std::string sub;
                iss >> sub;
                if (sub == "add") {
                    std::string router, netw, next;
                    iss >> router >> netw >> next;
                    net.routeAdd(router, netw, next);
                } else if (sub == "show") {
                    std::string router;
                    iss >> router;
                    net.routeShow(router);
                } else {
                    std::cout << "Usage: route add <router> <network> <next_hop|direct>  or  route show <router>\n";
                }
            }
            else if (cmd == "show") {
                net.showTopology();
            }
            else if (cmd == "exit" || cmd == "quit") {
                break;
            }
            else {
                std::cout << "Unknown command.\n";
            }
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}