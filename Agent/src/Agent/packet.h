/*
 * packet.h
 *
 *  Created on: Jan 13, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_PACKET_H_
#define SRC_AGENT_PACKET_H_

#include <netinet/ip.h>
#include <netinet/tcp.h>

typedef struct _packet
{
	struct nfqnl_msg_packet_hdr nfq_header;
	struct iphdr ip_header;
	struct tcphdr tcp_header;
	unsigned int dataLength;
	unsigned char* data;
	time_t timestamp;
} Packet;

#endif /* SRC_AGENT_PACKET_H_ */
