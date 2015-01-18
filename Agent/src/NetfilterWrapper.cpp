/*
 * NetfilterWrapper.cpp
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <string>
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
#include <regex>

#include "AgentTypes.h"

using namespace std;

NetfilterWrapper::NetfilterWrapper(std::shared_ptr<BlockingQueue<std::shared_ptr<Packet>>> tcpPacketsQueue, int queueNumber) :
		tcpPacketsQueue(tcpPacketsQueue), qh(0), fd(0), rv(0), queueNumber(queueNumber), worker(0) {

	clog << "Opening Netfilter Library handle... ";
	this->h = nfq_open();
	if (!h) {
		throw runtime_error(strerror(errno));
	}

	clog << "OK" << endl;
}

void* NetfilterWrapper::copy() {
	while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
		nfq_handle_packet(h, buf, rv);
	}

	return 0;
}

void* NetfilterWrapper::copyHelper(void* ctx) {
	return ((NetfilterWrapper*) ctx)->copy();
}

NetfilterWrapper::~NetfilterWrapper() {
	stop();
}

pthread_t NetfilterWrapper::start() {
	clog << "Staring Netfilter." << endl;
	clog << "Unbinding existing nf_queue handler for AF_INET (if any)... ";
	if (nfq_unbind_pf(h, AF_INET) < 0) {
		throw runtime_error(strerror(errno));
	}
	clog << "OK" << endl;

	clog << "Binding nf_queue handler for AF_INET... ";
	if (nfq_bind_pf(h, AF_INET) < 0) {
		throw runtime_error(strerror(errno));
	}
	clog << "OK" << endl;

	clog << "Binding handle to queue " << queueNumber << "... ";
	qh = nfq_create_queue(h, queueNumber, &NetfilterWrapper::netfilterQueueHandlerHelper, (void*)this);
	if (!qh) {
		throw runtime_error(strerror(errno));
	}
	clog << "OK" << endl;

	clog << "Setting copy packet mode... ";
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		throw runtime_error(strerror(errno));
	}
	clog << "OK" << endl;

	fd = nfq_fd(h);

	clog << "Creating thread... ";
	if (pthread_create(&worker, NULL, &NetfilterWrapper::copyHelper, this)
			!= 0) {
		stop();
		throw runtime_error(strerror(errno));
	}
	clog << "OK" << endl;

	clog << "Netfilter started." << endl;

	return worker;
}

int NetfilterWrapper::netfilterQueueHandler(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
		struct nfq_data *nfa) {
	unsigned char* payload;
	int payloadSize = nfq_get_payload(nfa, &payload);
	struct nfqnl_msg_packet_hdr *ph;
	ph = nfq_get_msg_packet_hdr(nfa);
	int id = ntohl(ph->packet_id);

	if (payloadSize > 0) {
		std::shared_ptr<Packet> packet(new Packet());
		packet->ip_header = *((struct iphdr *) payload);
		packet->tcp_header = *(struct tcphdr*) (payload
				+ sizeof(packet->ip_header));
		packet->nfq_header = *ph;
		int ipTcpHeaderLength = (sizeof(packet->ip_header)
				+ (packet->tcp_header.doff * 4));
		packet->dataLength = payloadSize - ipTcpHeaderLength;
		packet->data = (unsigned char *) (payload + ipTcpHeaderLength);

		timeval packetTime;
		if (nfq_get_timestamp(nfa, &packetTime) != 0) {
			packetTime.tv_sec = time(0); // timestap niezawsze jest ustawiany
		}
		packet->timestamp = packetTime.tv_sec;

		if (packet->dataLength > 0) {
			tcpPacketsQueue->add(packet);
		}
	}

	return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

int NetfilterWrapper::netfilterQueueHandlerHelper(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
		struct nfq_data *nfa, void *data)
{
	return ((NetfilterWrapper*)data)->netfilterQueueHandler(qh, nfmsg, nfa);
}

void NetfilterWrapper::stop() {
	clog << "Stopping Netfilter." << endl;

	close(fd);
	if(qh != 0)
	{
		nfq_destroy_queue(qh);
	}

	if(h != 0)
	{
		nfq_close(h);
	}

	clog << "Netfilter stopped." << endl;
}
