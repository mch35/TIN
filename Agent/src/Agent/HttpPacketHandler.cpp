/*
 * HttpPacketHandler.cpp
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#include "HttpPacketHandler.h"
#include "EnumParser.h"
#include <map>
#include <regex>
#include <netinet/ip.h>
#include <netinet/tcp.h>

HttpPacketHandler::HttpPacketHandler(
		std::shared_ptr<BlockingQueue<std::shared_ptr<Packet>>> tcpQueue) : tcpPacketsQueue(tcpQueue), startTime(0), stopTime(0), handlerThread(0) {

	pthread_mutex_init(&timeAccess,0);

}

HttpPacketHandler::~HttpPacketHandler() {

	pthread_mutex_destroy(&timeAccess);
	stop();
}

void* HttpPacketHandler::handleTcpPackets() {
	EnumParser<HttpMethod> enumParser;
	regex txt_regex("^([a-zA-Z]+)");
	std::cmatch m;
	while (1) {
		std::shared_ptr<Packet> tcpPacket = tcpPacketsQueue->get();

		pthread_mutex_lock(&timeAccess);
		time_t packetTime = tcpPacket->timestamp;

		if (isInTime(packetTime)) {
			if (std::regex_search((char*) tcpPacket->data, m, txt_regex)) {
				try {

					request_data data;
					data.method = enumParser.parse(m.str(0));
					data.receiver_ip.s_addr = ntohl(tcpPacket->ip_header.daddr);
					data.time = packetTime;

					std::cout << "method: " << data.method << std::endl;

					httpPackets.push_back(data);
				} catch (const std::runtime_error& e) {
					std::clog << "Not a HTTP packet!" << std::endl;
				}
			}
		} else {
			if (!dataReady.load() && packetTime < stopTime) {
				dataReady.store(true);
			}
		}
		// free memory
		tcpPacket = 0;
		pthread_mutex_unlock(&timeAccess);
	}

	return 0;
}

bool HttpPacketHandler::isInTime(time_t packetTime) {

	if (startTime <= packetTime && packetTime <= stopTime) {
		return true;
	}

	return false;
}

void* HttpPacketHandler::handleTcpPacketsHelper(void* ctx) {
	return ((HttpPacketHandler*) ctx)->handleTcpPackets();
}

vector<std::shared_ptr<request_data>>* HttpPacketHandler::getData() {
	pthread_mutex_lock(&timeAccess);
	vector<std::shared_ptr<request_data>>* copy = new vector<
			std::shared_ptr<request_data>>();

	for (request_data requestData : httpPackets) {
		copy->push_back(
				std::shared_ptr<request_data>(new request_data(requestData)));
	}

	pthread_mutex_unlock(&timeAccess);

	return copy;
}

void HttpPacketHandler::setTimeBounds(time_t startTime, time_t stopTime) {
	if (startTime > stopTime) {
		throw logic_error("Start time have to be less than stop time!");
	}

	// blokuje zeby wszystkie pakiety od teraz zalapaly sie do nowego pomiaru
	pthread_mutex_lock(&timeAccess);
	if (startTime < time(0)) {
		pthread_mutex_unlock(&timeAccess);
		throw logic_error("Start time have to be greater than current time!");
	}

	this->startTime = startTime;
	this->stopTime = stopTime;
	this->httpPackets.clear();
	this->dataReady.store(false);

	std::cout << "Starting new filtering. Start: " << ctime(&startTime)
			<< " Stop: " << ctime(&stopTime) << std::endl;
	pthread_mutex_unlock(&timeAccess);
}

pthread_t HttpPacketHandler::start() {
	if (pthread_create(&handlerThread, NULL,
			&HttpPacketHandler::handleTcpPacketsHelper, this) != 0) {
		stop();
		throw std::runtime_error("Error while creating new thread!");
	}

	return handlerThread;
}

void HttpPacketHandler::stop() {
}
