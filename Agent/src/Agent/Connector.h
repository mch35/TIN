/*
 * Connector.h
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_CONNECTOR_H_
#define SRC_AGENT_CONNECTOR_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <vector>
#include "HttpPacketHandler.h"
#include "communication.h"
#include "AgentExceptions.h"

/**
 * Pozwala na komunikację z serwerem.
 *
 */
class Connector
{
private:
	pthread_t listener;
	in_addr_t serverAddress;
	unsigned int serverPort;

	int serverSocket;

	void listeningThreadBody();
	static void* listeningThreadBodyHelper(void*);

	ClientResponse start(time_t when);
	ClientResponse stop(time_t when);
	ClientResponse get_data();
	vector<std::shared_ptr<request_data>>* get_data_records();

	// !!! STUB !!! tylko do testowania !!!
	HttpPacketHandler* agent;
public:
	Connector(HttpPacketHandler* agent, const char* serverAddress, const unsigned int serverPort);
	virtual ~Connector();

	/**
	 * Rozpoczyna nasluchiwanie na danym porcie
	 *
	 */
	void start() throw(std::runtime_error);

	/**
	 * Zatrzymuje nasluchiwanie na danym porcie
	 *
	 */
	void stop();
};

#endif /* SRC_AGENT_CONNECTOR_H_ */
