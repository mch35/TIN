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

using namespace std;

HttpPacketHandler::HttpPacketHandler(
		shared_ptr<BlockingQueue<shared_ptr<Packet>>> tcpQueue) : tcpPacketsQueue(tcpQueue), startTime(0), stopTime(0), handlerThread(0) {

	pthread_mutex_init(&timeAccess,0);

}

HttpPacketHandler::~HttpPacketHandler() {

	stop();
	pthread_mutex_destroy(&timeAccess);
}

bool HttpPacketHandler::tryMatch(const shared_ptr<Packet>& tcpPacket,
		const regex& txt_regex, cmatch& m) {
	return regex_search((char*) (tcpPacket->data), m, txt_regex);
}

void* HttpPacketHandler::handleTcpPackets() {
	EnumParser<HttpMethod> enumParser;
	regex requestRegex("^([a-zA-Z]+)");
	regex responseRegex("^HTTP/[0-9].[0-9] ([0-9]+) ");
	cmatch m;
	while (running.load()) {
		shared_ptr<Packet> tcpPacket = tcpPacketsQueue->get();

		pthread_mutex_lock(&timeAccess);
		time_t packetTime = tcpPacket->timestamp;

		if (isInTime(packetTime)) {
			try {
				request_data data;
				bool matched = false;

				if (matched = tryMatch(tcpPacket, responseRegex, m))
				{
					int responseCode = atoi(m.str(1).c_str());
					data.method = HttpMethod::DELETE; // TODO
				}
				else if (matched = tryMatch(tcpPacket, requestRegex, m)) {
					data.method = enumParser.parse(m.str(0));

				}

				if(matched)
				{
					data.receiver_ip.s_addr = ntohl(tcpPacket->ip_header.daddr);
					data.time = packetTime;
					httpPackets.push_back(data);
				}
			} catch (const std::runtime_error& e) {
				clog << "Not a HTTP packet! " << e.what() << std::endl;
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
	vector<shared_ptr<request_data>>* copy = new vector<
			shared_ptr<request_data>>();

	for (request_data requestData : httpPackets) {
		copy->push_back(
				shared_ptr<request_data>(new request_data(requestData)));
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

	clog << "> Starting new filtering.\n"
		 << "> Start: " << ctime(&(this->startTime))
		 << "> Stop: " << ctime(&(this->stopTime)) << endl;
	pthread_mutex_unlock(&timeAccess);
}

pthread_t HttpPacketHandler::start() {
	clog << "Starting http packets handler." << endl;
	running.store(true);
	clog << "Creating new thread... ";
	if (pthread_create(&handlerThread, NULL,
			&HttpPacketHandler::handleTcpPacketsHelper, this) != 0) {
		stop();
		throw std::runtime_error("Error while creating new thread!");
	}
	clog << "OK" << endl;
	clog << "Http packets handler started." << endl;

	return handlerThread;
}

void HttpPacketHandler::stop() {
	clog << "Stopping http packets handler." << endl;
	running.store(false);
	clog << "Http packets handler stopped." << endl;
}
