client: client.cpp serialization.cpp client_func.cpp communication.h client.h
	g++ client.cpp serialization.cpp client_func.cpp -o client -std=c++11
	
server: server.cpp serialization.cpp server.h communication.h 
	g++ server.cpp serialization.cpp -o server -std=c++11 -pthread -lpthread
