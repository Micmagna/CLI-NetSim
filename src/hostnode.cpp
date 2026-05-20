#include "hostnode.h"
#include <iostream>

HostNode::HostNode(const std::string& n) : Node(n) {}
std::string HostNode::getType() const { return "host"; }

void HostNode::processIncomingPacket(const Packet& pkt) {
    switch (pkt.protocol) {
        case 1: handleICMP(pkt); break;
        case 253:
            std::cout << name << " riceve da " << pkt.src.toString()
                      << ": " << pkt.payload << "\n";
            break;
        default:
            std::cout << name << " riceve pacchetto con protocollo sconosciuto ("
                      << (int)pkt.protocol << ")\n";
    }
}

void HostNode::handleICMP(const Packet& pkt) {
    switch (pkt.icmp_type) {
        case 8: // Echo Request
            {
                Packet reply(ip, pkt.src, 0, 0, pkt.icmp_id, pkt.icmp_seq);
                std::cout << name << " invia Echo Reply a " << pkt.src.toString() << "\n";
                sendPacket(reply);
            }
            break;
        case 0: // Echo Reply
            std::cout << name << " riceve Echo Reply da " << pkt.src.toString()
                      << " (id=" << pkt.icmp_id << ", seq=" << pkt.icmp_seq << ")\n";
            setICMPEvent(pkt.src, 0);
            break;
        case 11: // Time Exceeded
            std::cout << name << " riceve ICMP Time Exceeded da " << pkt.src.toString() << "\n";
            setICMPEvent(pkt.src, 11);
            break;
        case 3: // Destination Unreachable
            std::cout << name << " riceve ICMP Destination Unreachable (code="
                      << (int)pkt.icmp_code << ") da " << pkt.src.toString() << "\n";
            setICMPEvent(pkt.src, 3);
            break;
        default:
            std::cout << name << " riceve ICMP tipo " << (int)pkt.icmp_type
                      << " da " << pkt.src.toString() << "\n";
    }
}