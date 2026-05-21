# CLI-NetSim

A simple, educational network simulator written in C++17. It lets you create nodes (hosts, routers, and firewalls), assign IP addresses, connect them, and test connectivity via commands like `ping`, `traceroute`, and `send`. The simulator features **dynamic routing** using a Distance Vector algorithm, so routers learn routes automatically without manual configuration. Nodes can have multiple interfaces, enabling realistic network topologies. A **firewall node** with configurable rules (iptables‑like) allows filtering traffic based on protocol, source and destination IP.

## Features

- **Node types** – host, router, and firewall (extensible).
- **Multiple interfaces per node** – each node can have several network interfaces (e.g., `eth0`, `eth1`) with independent IP addresses.
- **IP addressing** – CIDR notation (e.g., `192.168.1.1/24`).
- **Point-to-point links** – connect two interfaces with `connect`.
- **Communication**:
  - Text messages (`send`)
  - Simulated ICMP Echo Request/Reply (`ping`)
  - Traceroute (`traceroute`)
- **Dynamic routing** – routers exchange routing tables automatically with `route update` (Distance Vector with split horizon). No static routes needed.
- **Firewall with configurable rules** – add, remove, and list filtering rules (allow/deny) based on protocol (`icmp`, `tcp`, `udp`, etc.) and source/destination IP.
- **Interactive shell**:
  - Tab‑completion for commands and node names.
  - Command history (saved to `~/.netsim_history`), navigable with arrow keys.
  - Built‑in `help` command.
  - Supports batch scripts (e.g., `./netsim < commands.txt`) and inline comments (`#`).

## Requirements

- **C++17 compiler** (g++ ≥ 7, clang++ ≥ 5)
- **GNU Readline library** (for line editing and completion)
  - Debian/Ubuntu: `sudo apt install libreadline-dev`
  - Fedora: `sudo dnf install readline-devel`
  - macOS: `brew install readline`
- **Make** (optional, to use the provided Makefile)

## Build

The source code is organised into `src/` and `include/`.

```bash
make clean && make
```
The executable is named netsim

## Usage

Launch the simulation

```bash
./netsim
```

Type help for available commands

Example session (Routing with interfacese):

```text
> add node PC1 host
> add node PC2 host
> add node R1 router
> set ip PC1 eth0 10.0.0.2/24
> set ip R1 eth0 10.0.0.1/24
> set ip R1 eth1 10.0.1.1/24
> set ip PC2 eth0 10.0.1.2/24
> connect PC1:eth0 R1:eth0
> connect R1:eth1 PC2:eth0
> route update
> ping PC1 PC2
PC1 sends ICMP Echo Request to PC2 (seq=1)
PC1 forwards packet to router R1
PC2 sends Echo Reply ...
PC1 receives Echo Reply from 10.0.1.2/24
> traceroute PC1 PC2
traceroute to 10.0.1.2/24 from PC1
1: 10.0.0.1/24
2: 10.0.1.2/24 (destination reached)
```

Example session (Firewall + Input from file)

Input:

```text
add node PC1 host
add node PC2 host
add node FW firewall
set ip PC1 eth0 10.0.0.2/24
set ip FW eth0 10.0.0.1/24
set ip FW eth1 10.0.1.1/24
set ip PC2 eth0 10.0.1.2/24
connect PC1:eth0 FW:eth0
connect FW:eth1 PC2:eth0

# 1. No rules, everything goes through
ping PC1 PC2               

# 2. Block ICMP from PC1 to PC2
firewall FW add rule deny proto icmp src 10.0.0.2 dst 10.0.1.2
ping PC1 PC2               # now fails

# 3. Show rules
firewall FW list

# 4. Remove deny rule
firewall FW remove rule 1
ping PC1 PC2               # works again

# 5. Add allow for traffic in both directions
firewall FW add rule allow proto icmp src 10.0.0.2 dst 10.0.1.2
firewall FW add rule allow proto icmp src 10.0.1.2 dst 10.0.0.2
ping PC1 PC2               # works
```

Output:

```text
Node PC1 of type host created.
Node PC2 of type host created.
Node FW of type firewall created.
Set IP 10.0.0.2/24 on PC1:eth0
Set IP 10.0.0.1/24 on FW:eth0
Set IP 10.0.1.1/24 on FW:eth1
Set IP 10.0.1.2/24 on PC2:eth0
Connected PC1:eth0 to FW:eth0
Connected FW:eth1 to PC2:eth0
PC1 sends ICMP Echo Request to PC2 (seq=1)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=1)
PC1 sends ICMP Echo Request to PC2 (seq=2)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=2)
PC1 sends ICMP Echo Request to PC2 (seq=3)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=3)
PC1 sends ICMP Echo Request to PC2 (seq=4)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=4)
Rule added to FW.
PC1 sends ICMP Echo Request to PC2 (seq=1)
PC1 forwards packet to firewall FW
FW (firewall): packet blocked by rule.
PC1 sends ICMP Echo Request to PC2 (seq=2)
PC1 forwards packet to firewall FW
FW (firewall): packet blocked by rule.
PC1 sends ICMP Echo Request to PC2 (seq=3)
PC1 forwards packet to firewall FW
FW (firewall): packet blocked by rule.
PC1 sends ICMP Echo Request to PC2 (seq=4)
PC1 forwards packet to firewall FW
FW (firewall): packet blocked by rule.
Firewall rules for FW:
1: deny proto 1 src 10.0.0.2/32 dst 10.0.1.2/32
Rule 1 removed from FW.
PC1 sends ICMP Echo Request to PC2 (seq=1)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=1)
PC1 sends ICMP Echo Request to PC2 (seq=2)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=2)
PC1 sends ICMP Echo Request to PC2 (seq=3)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=3)
PC1 sends ICMP Echo Request to PC2 (seq=4)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=4)
Rule added to FW.
Rule added to FW.
PC1 sends ICMP Echo Request to PC2 (seq=1)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=1)
PC1 sends ICMP Echo Request to PC2 (seq=2)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=2)
PC1 sends ICMP Echo Request to PC2 (seq=3)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=3)
PC1 sends ICMP Echo Request to PC2 (seq=4)
PC1 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.0.2/24 to 10.0.1.2/24
PC2 sends Echo Reply to 10.0.0.2/24
PC2 forwards packet to firewall FW
FW (firewall) forwards packet from 10.0.1.2/24 to 10.0.0.2/24
PC1 receives Echo Reply from 10.0.1.2/24 (id=4660, seq=4)
```

## Batch/Script Mode

You can pipe a file with commands (one per line) or use redirection:

```bash
./netsim < my_script.txt
```
Lines starting with # are treated as comments and ignored. Inline comments are also supported.

## License

This project is released under the MIT License (see LICENSE file).
