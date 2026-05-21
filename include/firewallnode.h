#ifndef FIREWALLNODE_H
#define FIREWALLNODE_H

#include "node.h"
#include <vector>

struct FirewallRule {
    enum Action { ALLOW, DENY };
    Action action;
    uint8_t protocol;   // 0 = any
    IPAddress src;      // 0.0.0.0/0 = any
    IPAddress dst;      // 0.0.0.0/0 = any

    FirewallRule() : action(DENY), protocol(0), src("0.0.0.0/0"), dst("0.0.0.0/0") {}

    bool matches(const Packet& pkt) const;
};

class FirewallNode : public Node {
    std::vector<FirewallRule> rules;
public:
    FirewallNode(const std::string& n);

    std::string getType() const override { return "firewall"; }

    // Apply firewall rules and forward packet between interfaces (bridge mode)
    void handleNonLocalPacket(const Packet& pkt) override;
    void processIncomingPacket(const Packet& pkt) override;

    // Manage rules
    void addRule(const FirewallRule& rule);
    void removeRule(int index);   // 1-based index
    const std::vector<FirewallRule>& getRules() const { return rules; }
};

#endif