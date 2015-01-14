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
#include "packet.h"

using namespace std;

std::shared_ptr<BlockingQueue<std::shared_ptr<Packet>>> _internalNetfilterQueue(new BlockingQueue<std::shared_ptr<Packet>>);

NetfilterWrapper::NetfilterWrapper(int queueNumber) :
		rv(0), queueNumber(queueNumber), worker(0) {

	clog << "Opening Netfilter Library handle\n";
	this->h = nfq_open();
	if (!h) {
		throw runtime_error("Error while opening Netfilter Library!");
	}

	clog << "Unbinding existing nf_queue handler for AF_INET (if any)\n";
	if (nfq_unbind_pf(h, AF_INET) < 0) {
		throw runtime_error("Error while existing queue handlers unbinding!\n");
	}

	clog << "Binding nf_queue handler for AF_INET\n";
	if (nfq_bind_pf(h, AF_INET) < 0) {
		throw runtime_error("Error while binding new queue handler!\n");
	}

	clog << "Binding handle to queue nr: " << queueNumber << "\n";
	qh = nfq_create_queue(h, queueNumber, &callback, NULL);
	if (!qh) {
		throw runtime_error(
				"Error while binding handle to queue nr: " + to_string(queueNumber) + "\n");
	}

	clog << "Setting copy packet mode\n";
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		throw runtime_error("Can't set copy packet mode\n");
	}

	fd = nfq_fd(h);

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
	if (pthread_create(&worker, NULL, &NetfilterWrapper::copyHelper, this)
			!= 0) {
		stop();
		throw runtime_error("Error while creating new thread!");
	}

	return worker;
}

int callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
		struct nfq_data *nfa, void *data) {
	unsigned char* payload;
	int payloadSize = nfq_get_payload(nfa, &payload);
	struct nfqnl_msg_packet_hdr *ph;
	ph = nfq_get_msg_packet_hdr(nfa);
	int id = ntohl(ph->packet_id);
	try {
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
				throw std::runtime_error(strerror(errno));
			}
			packet->timestamp = packetTime.tv_sec;
			/*
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

			 std::cout << "\n=============================\n";
			 char buf[4096];
			 nfq_snprintf_xml(buf, sizeof(buf), nfa, NFQ_XML_PAYLOAD);
			 std::cout << buf;
			 std::cout << "\n=============================\n";
			 }
			 */
			if (packet->dataLength > 0) {
				_internalNetfilterQueue->add(packet);
			}

		}
	} catch (const std::runtime_error& e) {
		std::clog << "Corrupted packet!\n";
	}

	return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

void NetfilterWrapper::stop() {
	close(fd);
	nfq_destroy_queue(qh);
	nfq_close(h);
}
