#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include "network.h"
#include <readline/readline.h>
#include <readline/history.h>

// Global variables for node names and current network (for completion)
static std::vector<std::string> nodeNames;
static Network* currentNetwork = nullptr;


std::vector<std::string> getNodeNames() {
    return nodeNames;
}

void updateNodeNames(Network& net) {
    nodeNames = net.getNodeNames();
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
                    static std::vector<std::string> opts = {"add", "show"};
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

    rl_attempted_completion_function = commandCompletion;

    const char* histfile = "~/.netsim_history";
    read_history(histfile);

    std::cout << "Network Simulator (C++ packet tracer light)\n";
    std::cout << "Commands:\n"
              << "  add node <name> [host|router]\n"
              << "  set ip <name> <IP/prefix>\n"
              << "  connect <A> <B>\n"
              << "  send <A> <B> <message>\n"
              << "  ping <A> <B> [count]\n"
              << "  traceroute <A> <B>\n"
              << "  route add <router> <network> <next_hop|direct>\n"
              << "  route show <router>\n"
              << "  show\n"
              << "  exit\n"
              << "TAB = complete, ↑↓ = history\n";

    char* input;
    while ((input = readline("> ")) != nullptr) {
        if (*input == 0) {
            free(input);
            continue;
        }

        if (history_length == 0 || history_get(history_length)->line != std::string(input)) {
            add_history(input);
        }

        std::string line(input);
        free(input);

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
                updateNodeNames(net);
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
                    std::cout << "Usage: route add <router> <network> <next_hop|direct> or route show <router>\n";
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

        updateNodeNames(net);
    }

    write_history(histfile);
    return 0;
}