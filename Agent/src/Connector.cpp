/*
 * Connector.cpp
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#include "Connector.h"
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

using namespace std;

Connector::Connector(HttpPacketHandler* agent, in_addr serverAddress, const unsigned int serverPort) :
		connectorThread(0), serverAddress(serverAddress), serverPort(
				serverPort), serverSocket(0), agent(agent) {
}

void Connector::listeningThreadBody() {
	int num = 0;
	unsigned char buffer[COMMAND_LENGTH], response[1];

	while (1) {
		num = recv(serverSocket, buffer, COMMAND_LENGTH, 0);

		if (num <= 0) {
			std::clog << "Connection closed." << std::endl;
			break;
		}
		command c = deserialize_command(buffer);
		std::clog << "Received command " << c.type << std::endl;
		switch (c.type) {
		case START:
			response[0] = (unsigned char) start(c.time);
			break;
		case STOP:
			response[0] = (unsigned char) stop(c.time);
			break;
		case GET_DATA:
			response[0] = (unsigned char) get_data();
			break;
		default: std::clog << "Unsupported command received!" << std::endl;
		}

		if ((send(serverSocket, response, 1, 0)) == -1) {
			cout << "Failure sending message " << strerror(errno) << endl;
			close (serverSocket);
			exit(1);
		}
		if (response[0] == 1 && c.type == GET_DATA) {
			vector<std::shared_ptr<request_data>>* data = get_data_records();
			unsigned char* s_size = serialize_int(data->size());
			if ((send(serverSocket, s_size, INT_LENGTH, 0)) == -1) {
				cout << "Failure sending message " << strerror(errno) << endl;
				close (serverSocket);
				exit(1);
			}
			delete (s_size);
			unsigned char* s_data;
			for (std::shared_ptr<request_data> requestData : *data)
			{
				s_data = serialize_request(*requestData.get());
				if ((send(serverSocket, s_data, R_D_LENGTH, 0)) == -1) {

					cout << "Failure sending message " << strerror(errno) << endl;
					close (serverSocket);
					exit(1);
				}
				delete (s_data);
				requestData = 0;
			}

			delete data;
		}
	}
}

void* Connector::listeningThreadBodyHelper(void* ctx)
{
	((Connector*) ctx)->listeningThreadBody();

	return 0;
}

Connector::~Connector() {
	stop();
}

pthread_t Connector::start() throw(std::runtime_error) {
	struct sockaddr_in serv_addr;
	clog << "Starting connector." << endl;

	clog << "Creating socket... ";
	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		throw std::runtime_error(strerror(errno));
	}
	clog << "OK" <<  endl;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(serverPort);
	serv_addr.sin_addr = serverAddress;

	clog << "Connecting to server " << inet_ntoa(serv_addr.sin_addr) << ":" << serverPort <<"... ";
	if (connect(serverSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
			< 0) {
		throw std::runtime_error(strerror(errno));
	}
	clog << "OK" <<  endl;

	clog << "Creating thread... ";
	if(pthread_create(&connectorThread, 0, &Connector::listeningThreadBodyHelper, this) != 0)
	{
		throw std::runtime_error(strerror(errno));
	}
	clog << "OK" <<  endl;

	clog << "Connector started." << endl;

	return connectorThread;
}

void Connector::stop()
{
	clog << "Stopping connector" << endl;

	connectorThread = 0;
	serverSocket = 0;
	close(serverSocket);

	clog << "Connector stopped." << endl;
}

time_t startTime;
time_t stopTime;

ClientResponse Connector::start(time_t when) {
	startTime = time(0) + 10;
	return OK;
}
ClientResponse Connector::stop(time_t when) {
	stopTime = startTime + 1000000;
	try
	{
		agent->setTimeBounds(startTime, stopTime);
	}
	catch(exception& e)
	{
		return ERROR;
	}

	return OK;
}
ClientResponse Connector::get_data() {
	if(agent->isDataReady())
	{
		return OK;
	}

	std::clog << "Data not ready yet!" << std::endl;

	return ERROR;
}
vector<std::shared_ptr<request_data>>* Connector::get_data_records() {
	auto packets = agent->getData();
	std::clog << "Prepared " << packets->size() << " packets." << std::endl;
	return packets;
}
