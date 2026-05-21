#include "hostnode.h"
#include <iostream>

HostNode::HostNode(const std::string& n) : Node(n) {}

std::string HostNode::getType() const { return "host"; }

void HostNode::processIncomingPacket(const Packet& pkt) {
    switch (pkt.protocol) {
        case 1: handleICMP(pkt); break;
        case 253:
            std::cout << name << " receives from " << pkt.src.toString()
                      << ": " << pkt.payload << "\n";
            break;
        default:
            std::cout << name << " receives packet with unknown protocol ("
                      << (int)pkt.protocol << ")\n";
    }
}

void HostNode::handleICMP(const Packet& pkt) {
    IPAddress myIP = getFirstIP();   // use first available IP for replies
    switch (pkt.icmp_type) {
        case 8: // Echo Request
            {
                Packet reply(myIP, pkt.src, 0, 0, pkt.icmp_id, pkt.icmp_seq);
                std::cout << name << " sends Echo Reply to " << pkt.src.toString() << "\n";
                sendPacket(reply);
            }
            break;
        case 0: // Echo Reply
            std::cout << name << " receives Echo Reply from " << pkt.src.toString()
                      << " (id=" << pkt.icmp_id << ", seq=" << pkt.icmp_seq << ")\n";
            setICMPEvent(pkt.src, 0);
            break;
        case 11: // Time Exceeded
            std::cout << name << " receives ICMP Time Exceeded from " << pkt.src.toString() << "\n";
            setICMPEvent(pkt.src, 11);
            break;
        case 3: // Destination Unreachable
            std::cout << name << " receives ICMP Destination Unreachable (code="
                      << (int)pkt.icmp_code << ") from " << pkt.src.toString() << "\n";
            setICMPEvent(pkt.src, 3);
            break;
        default:
            std::cout << name << " receives ICMP type " << (int)pkt.icmp_type
                      << " from " << pkt.src.toString() << "\n";
    }
}