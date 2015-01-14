
OBJS=Agent.o HttpPacketHandler.o NetfilterWrapper.o Connector.o  serialization.o 
LIBS=-lnfnetlink -lmnl -lnetfilter_queue -lpthread -L/usr/local/lib
PARAMS= -std=c++11
CC=g++

all: Agent
	
Agent: Agent.cpp HttpPacketHandler.cpp NetfilterWrapper.cpp Connector.cpp serialization.cpp
	$(CC) $(PARAMS) Agent.cpp HttpPacketHandler.cpp NetfilterWrapper.cpp Connector.cpp serialization.cpp -o Agent $(LIBS)

NetfilterWrapper.o: NetfilterWrapper.cpp 
	$(CC) $(PARAMS) NetfilterWrapper.cpp -o NetfilterWrapper.o $(LIBS)

Connector.o: Connector.cpp serialization.cpp
	$(CC) $(PARAMS) Connector.cpp serialization.cpp -o Connector.o $(LIBS)
	
HttpPacketHandler.o: HttpPacketHandler.cpp
	$(CC) $(PARAMS) HttpPacketHandler.cpp -o HttpPacketHandler.o $(LIBS)
	
serialization.o: serialization.cpp
	$(CC) $(PARAMS) serialization.cpp -o serialization.o

clean:
	rm -f *.o  

.PHONY: all clean
