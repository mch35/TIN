/*
 * NetfilterWrapper.cpp
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

#include <libmnl/libmnl.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink.h>

#include <linux/types.h>
#include <linux/netfilter/nfnetlink_queue.h>

#include <libnetfilter_queue/libnetfilter_queue.h>

#include "NetfilterWrapper.h"
#include "BlockingQueue.h"
#include <pthread.h>
#include <iostream>
#include <ctime>
#include <memory>

#include "packet.h"

using namespace std;

std::shared_ptr<BlockingQueue<std::shared_ptr<Packet>>> _internalNetfilterQueue(new BlockingQueue<std::shared_ptr<Packet>>);

NetfilterWrapper::NetfilterWrapper(int queueNumber) : queueNumber(queueNumber) {
	this->openLibrary();
	this->unbindHandler();
	this->bindHandler();
	this->createQueue(&callback);
	this->setMode();

	fd = nfq_fd(h);

	pthread_create(&worker, NULL, &NetfilterWrapper::copyHelper, this);
}

void NetfilterWrapper::openLibrary()
{
	printf("opening library handle\n");
	this->h = nfq_open();
	if (!h) {
		fprintf(stderr, "error during nfq_open()\n");
		exit(1);
	}
}

void NetfilterWrapper::unbindHandler()
{
	printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
	if (nfq_unbind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nfq_unbind_pf()\n");
		exit(1);
	}
}

void NetfilterWrapper::bindHandler()
{
	printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
	if (nfq_bind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nfq_bind_pf()\n");
		exit(1);
	}
}

void NetfilterWrapper::createQueue(nfq_callback *cb)
{
	printf("binding this socket to queue '0'\n");
	qh = nfq_create_queue(h,  0, cb, NULL);
	if (!qh) {
		fprintf(stderr, "error during nfq_create_queue()\n");
		exit(1);
	}
}

void NetfilterWrapper::setMode()
{
	printf("setting packet mode\n");
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		fprintf(stderr, "can't set packet mode\n");
		exit(1);
	}
}

void* NetfilterWrapper::copy()
{
	while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
		nfq_handle_packet(h, buf, rv);
		//stąd jakoś pobierać rv, żeby później wiedzieć jakiej długości dane.
	}

	std::cout<<"closed================================";

	return 0;
}


void* NetfilterWrapper::copyHelper(void* ctx)
{
	return ((NetfilterWrapper*)ctx)->copy();
}

NetfilterWrapper::~NetfilterWrapper() {
	nfq_destroy_queue(qh);
	nfq_close(h);
}

pthread_t NetfilterWrapper::getThread()
{
	return worker;
}

time_t __netfilterStarTime = 0;
time_t __netfilterStopTime = 0;

bool isInTime(struct nfq_data *tcpPacket)
{
	timeval packetTime;
	nfq_get_timestamp(tcpPacket, &packetTime);
	time_t tm = packetTime.tv_sec;

	if(__netfilterStarTime < tm && tm < __netfilterStopTime)
	{
		return true;
	}

	return false;
}

int callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
	unsigned char* payload;
	int payloadSize = nfq_get_payload(nfa, &payload);

	try
	{
		if(payloadSize > 0)
		{
			std::shared_ptr<Packet> packet(new Packet());
			packet->ip_header = *((struct iphdr *)payload);
			packet->tcp_header = *(struct tcphdr*)(payload + sizeof(packet->ip_header));
			packet->nfq_header = *nfq_get_msg_packet_hdr(nfa);
			packet->dataLength = payloadSize - (sizeof(packet->ip_header) + (packet->tcp_header.doff * 4));
			packet->data = (char *)((unsigned char *)&(packet->tcp_header) + (packet->tcp_header.doff * 4));
			timeval packetTime;

			packet->timestamp = packetTime.tv_sec;

			{
				cout << "nfqheader: " << sizeof(packet->nfq_header) << endl;
				cout << "ipheader: " << sizeof(packet->ip_header) << endl;
				cout << "tcpheader: " << sizeof(packet->tcp_header) << endl;
				cout << "tcpOffset: " << (packet->tcp_header.doff * 4) << endl;

				cout << "payloadSize: " << payloadSize << endl;
				cout << "dataLength " << packet->dataLength << endl;
				in_addr addr;
				addr.s_addr = packet->ip_header.daddr;
				cout << inet_ntoa(addr) << endl;
/*
				std::cout << "\n=============================\n";
				char buf[4096];
				nfq_snprintf_xml(buf, sizeof(buf), nfa, NFQ_XML_ALL);
				std::cout << buf;
				std::cout << "\n=============================\n";*/
			}

			if(packet->dataLength > 0)
			{
				for(int i = 0; i < packet->dataLength; ++i)
				{
					printf("%c", packet->data + i);
				}
			}
			if(nfq_get_timestamp(nfa, &packetTime) != 0)
			{
				throw std::runtime_error(strerror(errno));
			}

			//_internalNetfilterQueue->add(packet);
		}
	}
	catch(const std::runtime_error& e)
	{
		std::clog << "Corrupted packet!\n";
	}
	timeval packetTime;
	nfq_get_timestamp(nfa, &packetTime);

	struct nfqnl_msg_packet_hdr *ph;
	ph = nfq_get_msg_packet_hdr(nfa);
	int id = ntohl(ph->packet_id);
	//std::cout << "id: " << id << " t: " << packetTime.tv_sec << std::endl;
/*
	char buf[4096];
	std::cout << "\n=============================\n";
	nfq_snprintf_xml(buf, sizeof(buf), nfa, NFQ_XML_ALL);
	std::cout << buf;
	delete nfa;*/
	return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

void NetfilterWrapper::stop()
{
	close(fd);
}
