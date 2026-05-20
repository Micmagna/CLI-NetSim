#include <iostream>
#include <sstream>
#include <string>
#include "network.h"

int main() {
    Network net;
    std::string line;

    std::cout << "Network Simulator (C++ Packet Tracer light)\n";
    std::cout << "Commands: add node <name> [type] | set ip <name> <IP/prefix> | "
                 "connect <A> <B> | send <A> <B> <message> | show | exit\n";

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
                    std::cout << "Usage: add node <name> [type]\n";
                    continue;
                }
                std::string nodeType = "host";
                if (!iss.eof()) {
                    iss >> nodeType;
                }
                net.addNode(name, nodeType);
            }
            else if (cmd == "set") {
                std::string what, name, ip;
                iss >> what >> name >> ip;
                if (what != "ip") {
                    std::cout << "Usage: set ip <name> <IP/prefix>\n";
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
            else if (cmd == "show") {
                net.showTopology();
            }
            else if (cmd == "exit" || cmd == "quit") {
                break;
            }
            else {
                std::cout << "Unknown command. Commands: add, set, connect, send, show, exit\n";
            }
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}