/*
 * Connector.h
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_CONNECTOR_H_
#define SRC_AGENT_CONNECTOR_H_

#include <vector>
#include <pthread.h>
#include <arpa/inet.h>
#include <memory>
#include "HttpPacketHandler.h"
#include "../../Utils/src/communication.h"

/**
 * Pozwala na komunikację z serwerem.
 *
 */
class Connector
{
private:
	pthread_t connectorThread;
	in_addr serverAddress;
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
	Connector(HttpPacketHandler* agent, in_addr serverAddress, const unsigned int serverPort);
	virtual ~Connector();

	/**
	 * Rozpoczyna nasluchiwanie na danym porcie
	 *
	 */
	pthread_t start() throw(std::runtime_error);

	/**
	 * Zatrzymuje nasluchiwanie na danym porcie
	 *
	 */
	void stop();
};

#endif /* SRC_AGENT_CONNECTOR_H_ */
