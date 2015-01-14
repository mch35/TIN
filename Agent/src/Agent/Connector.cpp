/*
 * Connector.cpp
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#include "Connector.h"

using namespace std;

Connector::Connector(HttpPacketHandler* agent, const char* serverAddress, const unsigned int serverPort) :
		listener(0), serverAddress(inet_addr(serverAddress)), serverPort(
				serverPort), serverSocket(0), agent(agent) {
}

void Connector::listeningThreadBody() {
	int num = 0;
	unsigned char buffer[COMMAND_LENGTH], response[1];

	while (1) {
		num = recv(serverSocket, buffer, COMMAND_LENGTH, 0);

		if (num <= 0) {
			cout << "Connection closed or error " << strerror(errno) << endl;
			break;
		}
		command c = deserialize_command(buffer);
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

	stop();
}

void* Connector::listeningThreadBodyHelper(void* ctx)
{
	((Connector*) ctx)->listeningThreadBody();

	return 0;
}

Connector::~Connector() {
	stop();
}

void Connector::start() throw(std::runtime_error) {
	struct sockaddr_in serv_addr;

	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		stop();
		throw std::runtime_error(strerror(errno));
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(serverPort);
	serv_addr.sin_addr.s_addr = serverAddress;

	if (connect(serverSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
			< 0) {
		stop();
		throw std::runtime_error(strerror(errno));
	}

	pthread_create(&listener, 0, &Connector::listeningThreadBodyHelper, this);
}

void Connector::stop()
{
	close(serverSocket);
	listener = 0;
	serverSocket = 0;
}

time_t startTime;
time_t stopTime;

ClientResponse Connector::start(time_t when) {
	startTime = time(0) + 10;
	return OK;
}
ClientResponse Connector::stop(time_t when) {
	stopTime = startTime + 5;
	try
	{
		agent->setStartTime(startTime, stopTime);
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
		cout << "data!" << endl;
		return OK;
	}

	cout << "no data!" << endl;
	return ERROR;
}
vector<std::shared_ptr<request_data>>* Connector::get_data_records() {
	return agent->getData();
}
