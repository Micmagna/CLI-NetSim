#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <algorithm>
#include "network.h"
#include <readline/readline.h>
#include <readline/history.h>

static std::vector<std::string> nodeNames;
static Network* currentNetwork = nullptr;

std::vector<std::string> getNodeNames() {
    return nodeNames;
}

void updateNodeNames(Network& net) {
    nodeNames = net.getNodeNames();
}

void printHelp() {
    std::cout << "Available commands:\n"
              << "  add node <name> [host|router]       – create a new node (default: host)\n"
              << "  set ip <name> [<interface>] <IP/prefix> – assign an IP address to a node\n"
              << "  connect <A>[:<ifA>] <B>[:<ifB>]     – connect two nodes\n"
              << "  send <A> <B> <message>              – send a text message\n"
              << "  ping <A> <B> [count]                – test connectivity with simulated ICMP\n"
              << "  traceroute <A> <B>                  – trace the path to a destination\n"
              << "  route show <router>                 – display the routing table\n"
              << "  route update                        – run dynamic routing (Distance Vector)\n"
              << "  show                                – show the current topology\n"
              << "  help                                – show this help\n"
              << "  exit / quit                         – exit the simulator\n";
}

char** commandCompletion(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;

    std::string currentLine(rl_line_buffer);
    std::istringstream iss(currentLine);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) tokens.push_back(token);

    if (start == 0) {
        return rl_completion_matches(text, [](const char* t, int state) -> char* {
            static std::vector<std::string> cmds = {
                "add", "set", "connect", "send", "ping",
                "traceroute", "route", "show", "exit", "quit"
            };
            static size_t idx;
            if (state == 0) idx = 0;
            while (idx < cmds.size()) {
                std::string c = cmds[idx++];
                if (c.find(t) == 0 && c != t) return strdup(c.c_str());
            }
            return nullptr;
        });
    }

    if (tokens.size() >= 1) {
        int token2_start = tokens[0].length() + 1;
        bool completing_second = (tokens.size() == 1 && start >= token2_start) ||
                                 (tokens.size() == 2 && start >= token2_start && 
                                  start <= token2_start + static_cast<int>(tokens[1].length()));
        if (completing_second) {
            if (tokens[0] == "add") {
                return rl_completion_matches(text, [](const char* t, int state) -> char* {
                    static std::vector<std::string> opts = {"node"};
                    static size_t idx;
                    if (state == 0) idx = 0;
                    while (idx < opts.size()) {
                        std::string o = opts[idx++];
                        if (o.find(t) == 0 && o != t) return strdup(o.c_str());
                    }
                    return nullptr;
                });
            }
            if (tokens[0] == "set") {
                return rl_completion_matches(text, [](const char* t, int state) -> char* {
                    static std::vector<std::string> opts = {"ip"};
                    static size_t idx;
                    if (state == 0) idx = 0;
                    while (idx < opts.size()) {
                        std::string o = opts[idx++];
                        if (o.find(t) == 0 && o != t) return strdup(o.c_str());
                    }
                    return nullptr;
                });
            }
            if (tokens[0] == "connect" || tokens[0] == "send" ||
                tokens[0] == "ping" || tokens[0] == "traceroute") {
                return rl_completion_matches(text, [](const char* t, int state) -> char* {
                    static size_t idx;
                    if (state == 0) idx = 0;
                    auto names = getNodeNames();
                    while (idx < names.size()) {
                        std::string n = names[idx++];
                        if (n.find(t) == 0 && n != t) return strdup(n.c_str());
                    }
                    return nullptr;
                });
            }
            if (tokens[0] == "route") {
                return rl_completion_matches(text, [](const char* t, int state) -> char* {
                    static std::vector<std::string> opts = {"show", "update"};
                    static size_t idx;
                    if (state == 0) idx = 0;
                    while (idx < opts.size()) {
                        std::string o = opts[idx++];
                        if (o.find(t) == 0 && o != t) return strdup(o.c_str());
                    }
                    return nullptr;
                });
            }
        }
    }

    return nullptr;
}

int main() {
    Network net;
    currentNetwork = &net;
    bool interactive = isatty(fileno(stdin));
    if (interactive) {
        rl_attempted_completion_function = commandCompletion;
        const char* histfile = "~/.netsim_history";
        read_history(histfile);
        std::cout << "Welcome to the Network Simulator!\nType 'help' for a list of commands.\n";
    }
    char* input = nullptr;
    while (true) {
        if (interactive) {
            input = readline("> ");
            if (!input) break;     
            if (*input) {
                if (history_length == 0 || history_get(history_length)->line != std::string(input))
                    add_history(input);
            }
        } else {
            std::string line;
            if (!std::getline(std::cin, line)) break;
            input = strdup(line.c_str());
        }

        if (*input == '\0') {
            free(input);
            continue;
        }
        std::string line(input);
        free(input);

        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
            line.pop_back();

        if (line.empty()) continue;

        if (line[0] == '#') continue;

        updateNodeNames(net);
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
                updateNodeNames(net);
            }
            else if (cmd == "set") {
                std::string what, name, ifname, ip;
                iss >> what >> name;
                if (what != "ip") { std::cout << "Usage: set ip ...\n"; continue; }
                // peek next token: if it contains '/', it's an IP, else it's an interface
                std::string next;
                auto pos = iss.tellg();
                if (iss >> next) {
                    if (next.find('/') != std::string::npos) {
                        ifname = "eth0";
                        ip = next;
                    } else {
                        ifname = next;
                        iss >> ip;
                    }
                } else {
                    std::cout << "Missing IP address.\n";
                    continue;
                }
                net.setNodeIP(name, ifname, ip);
            }
            else if (cmd == "connect") {
                std::string a, b;
                iss >> a >> b;
                auto split = [](const std::string& s, std::string& node, std::string& ifname) {
                    auto pos = s.find(':');
                    if (pos != std::string::npos) {
                        node = s.substr(0, pos);
                        ifname = s.substr(pos + 1);
                    } else {
                        node = s;
                        ifname = "eth0";
                    }
                };
                std::string nodeA, ifA, nodeB, ifB;
                split(a, nodeA, ifA);
                split(b, nodeB, ifB);
                net.connectNodes(nodeA, ifA, nodeB, ifB);
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
                if (sub == "show") {
                    std::string router;
                    iss >> router;
                    net.routeShow(router);
                } else if (sub == "update") {
                    net.routeUpdate();
                } else {
                    std::cout << "Usage: route [show <router> | update]\n";
                }
            }
            else if (cmd == "show") {
                net.showTopology();
            }
            else if (cmd == "exit" || cmd == "quit") {
                break;
            }
            else if (cmd == "help") {
                printHelp();
            }
            else {
                std::cout << "Unknown command.\n";
            }
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
        updateNodeNames(net);
    }

    if (interactive) {
        write_history("~/.netsim_history");
    }

    return 0;
}