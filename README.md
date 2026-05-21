# CLI-NetSim

A simple, educational network simulator written in C++17. It lets you create nodes (hosts and routers), assign IP addresses, connect them, and test connectivity via commands like `ping`, `traceroute`, and `send`. The simulator features **dynamic routing** using a Distance Vector algorithm, so routers learn routes automatically without manual configuration. Nodes can have multiple interfaces, enabling realistic network topologies.

## Features

- **Node types** – host and router (extensible).
- **Multiple interfaces per node** – each node can have several network interfaces (e.g., `eth0`, `eth1`) with independent IP addresses.
- **IP addressing** – CIDR notation (e.g., `192.168.1.1/24`).
- **Point-to-point links** – connect two interfaces with `connect`.
- **Communication**:
  - Text messages (`send`)
  - Simulated ICMP Echo Request/Reply (`ping`)
  - Traceroute (`traceroute`)
- **Dynamic routing** – routers exchange routing tables automatically with `route update` (Distance Vector with split horizon). No static routes needed.
- **Firewall-ready hook** – every node has a virtual `filterPacket()` method, ready to implement iptables-like rules.
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

Example session:

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

## Available Commands

| Command                                       | Description                                                                    |        
|:---------------------------------------------:|:------------------------------------------------------------------------------:|
| add node <name> <host|router>                 | Create node                                                                    | 
| set ip <name> <interface> <IP/prefix>         | Assign an IP address to a node. If interface is omitted, eth0 is used.         |   
| connect <A>:<ifA> <B>:<ifB>                   | Create a point‑to‑point link between two interfaces. Default interface is eth0.|   
| send <A> <B> <message>                        | Send txt message                                                               | 
| ping <A> <B> <count>                          | Simulated ICMP ping                                                            |   
| traceroute <A> <B>                            | Simulated ICMP traceroute                                                      |  
| route update                                  | Start dynamic routing                                                          | 
| route show <router>                           | Display routing table                                                          |  
| show                                          | Show net topology                                                              |
| help                                          | List of available commands                                                     |     
| exit/quit                                     | Quit application                                                               |  

## Batch/Script Mode

You can pipe a file with commands (one per line) or use redirection:

```bash
./netsim < my_script.txt
```
Lines starting with # are treated as comments and ignored.

## License

This project is released under the MIT License (see LICENSE file).
