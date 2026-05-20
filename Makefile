CXX = g++
CXXFLAGS = -std=c++17 -Wall
OBJS = main.o ipaddress.o node.o link.o hostnode.o network.o

netsim: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o netsim