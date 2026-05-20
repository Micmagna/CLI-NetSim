CXX = g++
CXXFLAGS = -std=c++17 -Wall
LDLIBS = -lreadline
OBJS = main.o ipaddress.o node.o link.o hostnode.o routernode.o network.o

netsim: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

main.o: main.cpp network.h
ipaddress.o: ipaddress.cpp ipaddress.h
node.o: node.cpp node.h link.h packet.h ipaddress.h
link.o: link.cpp link.h node.h packet.h
hostnode.o: hostnode.cpp hostnode.h node.h
routernode.o: routernode.cpp routernode.h node.h
network.o: network.cpp network.h hostnode.h routernode.h link.h node.h

clean:
	rm -f *.o netsim