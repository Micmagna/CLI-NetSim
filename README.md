# CLI-NetSim

A simple, educational network simulator written in C++17. It lets you create nodes (hosts and routers), assign IP addresses, connect them, and test connectivity via commands like `ping`, `traceroute`, and `send`. The goal is to provide a lightweight, interactive tool to understand networking concepts, similar to Cisco Packet Tracer but entirely on the command line.

## Features

- **Node types** – host and router (extensible).
- **IP addressing** – CIDR notation (e.g., `192.168.1.1/24`).
- **Point-to-point links** – connect two nodes with `connect`.
- **Communication**:
  - Text messages (`send`)
  - Simulated ICMP Echo Request/Reply (`ping`)
  - Traceroute (`traceroute`)
- **Routing** – routers maintain a routing table; direct routes are added automatically, and you can add manual routes (`route add`, `route show`).
- **Firewall-ready hook** – every node has a virtual `filterPacket()` method, ready to implement iptables-like rules.
- **Interactive shell**:
  - Tab‑completion for commands and node names.
  - Command history (saved to `~/.netsim_history`), navigable with arrow keys.

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

Example session:

```text
> add node PC1 host
> add node PC2 host
> add router R1
> set ip PC1 10.0.0.2/24
> set ip PC2 10.0.1.2/24
> set ip R1 10.0.0.1/24
> connect PC1 R1
> connect R1 PC2
> ping PC1 PC2
```

## Available Commands

| Command                                       | Description              |        
|:---------------------------------------------:|:------------------------:| 
| add node <name> [host|router]                 | Create node              | 
| set ip <name> <IP/Prefix>                     | Assign IP                |   
| connect <A> <B>                               | Link two nodes           |   
| send <A> <B> <message>                        | Send txt message         | 
| ping <A> <B> [count]                          | Simulated ICMP ping      |   
| traceroute <A> <B>                            | Simulated ICMP traceroute|  
| route add <router> <network> <next_hop|direct>| Add a route              | 
| route show <router>                           | Display routing table    |  
| show                                          | Show net topology        |    
| exit/quit                                     | Quit application         |  

## License

This project is released under the MIT License (see LICENSE file).