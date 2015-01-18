/*
 * packet.h
 *
 *  Created on: Jan 13, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_TYPES_H_
#define SRC_AGENT_TYPES_H_

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

typedef struct _AgentArgs
{
	in_addr i;
	unsigned int p;
	unsigned int q;
	bool h;

	_AgentArgs() : p(5000), q(0), h(false)
	{
		i.s_addr = inet_addr("127.0.0.1");
	}
} AgentArgs;

typedef struct _Packet
{
	struct nfqnl_msg_packet_hdr nfq_header;
	struct iphdr ip_header;
	struct tcphdr tcp_header;
	unsigned int dataLength;
	unsigned char* data;
	time_t timestamp;
} Packet;

#endif /* SRC_AGENT_TYPES_H_ */
