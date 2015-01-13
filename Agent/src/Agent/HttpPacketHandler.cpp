/*
 * HttpPacketHandler.cpp
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#include "HttpPacketHandler.h"
/*
#include <netinet/tcp.h>
#include <fstream>
#include <regex>*/

HttpPacketHandler::HttpPacketHandler(std::shared_ptr<BlockingQueue<nfq_data>> tcpQueue) : tcpPacketsQueue(tcpQueue), startTime(0), stopTime(0) {

	pthread_mutex_init(&timeAccess,0);
	pthread_create(&handlerThread, NULL, &HttpPacketHandler::handleTcpPacketsHelper, this);

}

HttpPacketHandler::~HttpPacketHandler() {

    pthread_mutex_destroy(&timeAccess);
}

time_t getPacketTime(nfq_data* tcpPacket)
{
	timeval packetTime;
	nfq_get_timestamp(tcpPacket, &packetTime);
	return packetTime.tv_sec;
}

void* HttpPacketHandler::handleTcpPackets()
{
	unsigned char* data;
	//char* tcpdata;
	int ret;

	while(1)
	{
		nfq_data* tcpPacket = tcpPacketsQueue->get();

		pthread_mutex_lock(&timeAccess);
		time_t packetTime = getPacketTime(tcpPacket);
		struct nfqnl_msg_packet_hdr *ph;
		ph = nfq_get_msg_packet_hdr(tcpPacket);
		uint32_t id = ntohl(ph->packet_id);
		cout << "handling " << "id: " << id << " time: " << packetTime << std::endl;

		if(isInTime(packetTime))
		{
			ret = nfq_get_payload(tcpPacket, &data);
			if(ret > 0)
			{
				std::cout << "caught!!!\n";
				struct iphdr * ip_info = (struct iphdr *)data;
				//struct tcphdr * tcp_info = (struct tcphdr*)(data + sizeof(*ip_info));
				//tcpdata = (char *)((unsigned char *)tcp_info + (tcp_info->doff * 4));

				uint32_t destIp = ntohl(ip_info->daddr);
				request_data data;
				data.method = HttpMethod::GET;
				data.receiver_ip.s_addr = destIp;
				data.time = packetTime;

				httpPackets.push_back(data);


				//std::regex txt_regex("^[a-zA-Z]+", std::regex_constants::basic);
				//std::cout << "match : " << std::regex_match(tcpdata, txt_regex) << '\n';

				//std::cout << "\n==============================\n";
			}
			else
			{
				std::cout << "ret: " << ret << std::endl;
			}
		}
		else
		{
			if(!dataReady.load() && packetTime < stopTime)
			{
				dataReady.store(true);
			}
		}
		tcpPacket = 0;
		pthread_mutex_unlock(&timeAccess);
	}

	return 0;
}

bool HttpPacketHandler::isInTime(time_t packetTime)
{

	if(startTime <= packetTime && packetTime <= stopTime)
	{
		return true;
	}

	return false;
}

void* HttpPacketHandler::handleTcpPacketsHelper(void* ctx)
{
	return ((HttpPacketHandler*) ctx)->handleTcpPackets();
}

vector<std::shared_ptr<request_data>>* HttpPacketHandler::getData()
{
	pthread_mutex_lock(&timeAccess);
	vector<std::shared_ptr<request_data>>* copy = new vector<std::shared_ptr<request_data>>();

	for(request_data requestData : httpPackets)
	{
		copy->push_back(std::shared_ptr<request_data>(new request_data(requestData)));
	}

	pthread_mutex_unlock(&timeAccess);

	return copy;
}

/*void HttpPacketHandler::startFiltering(time_t* startTime, time_t* stopTime)
{
	pthread_mutex_lock(&timeAccess);
	this->httpPackets.clear();
	this->startTime = *startTime;
	this->stopTime = *stopTime;

	std::cout << "starting: " << this->startTime << " to " << this->stopTime << " now " << time(0) << std::endl;
	pthread_mutex_unlock(&timeAccess);
}*/
