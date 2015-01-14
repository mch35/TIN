//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <netinet/in.h>
//#include <linux/types.h>
//#include <linux/netfilter.h>		/* for NF_ACCEPT */
//
//#include <libnetfilter_queue/libnetfilter_queue.h>
//
///* returns packet id */
//static u_int32_t print_pkt (struct nfq_data *tb)
//{
//	int id = 0;
//	struct nfqnl_msg_packet_hdr *ph;
//	struct nfqnl_msg_packet_hw *hwph;
//	u_int32_t mark,ifi;
//	int ret;
//	unsigned char *data;
//
//	ph = nfq_get_msg_packet_hdr(tb);
//	if (ph) {
//		id = ntohl(ph->packet_id);
//		printf("hw_protocol=0x%04x hook=%u id=%u ",
//			ntohs(ph->hw_protocol), ph->hook, id);
//	}
//
//	hwph = nfq_get_packet_hw(tb);
//	if (hwph) {
//		int i, hlen = ntohs(hwph->hw_addrlen);
//
//		printf("hw_src_addr=");
//		for (i = 0; i < hlen-1; i++)
//			printf("%02x:", hwph->hw_addr[i]);
//		printf("%02x ", hwph->hw_addr[hlen-1]);
//	}
//
//	mark = nfq_get_nfmark(tb);
//	if (mark)
//		printf("mark=%u ", mark);
//
//	ifi = nfq_get_indev(tb);
//	if (ifi)
//		printf("indev=%u ", ifi);
//
//	ifi = nfq_get_outdev(tb);
//	if (ifi)
//		printf("outdev=%u ", ifi);
//	ifi = nfq_get_physindev(tb);
//	if (ifi)
//		printf("physindev=%u ", ifi);
//
//	ifi = nfq_get_physoutdev(tb);
//	if (ifi)
//		printf("physoutdev=%u ", ifi);
//
//	ret = nfq_get_payload(tb, &data);
//	if (ret >= 0) {
//		printf("payload_len=%d ", ret);
//	}
//	fputc('\n', stdout);
//
//	return id;
//}
//
//static int i = 0;
//
//static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
//	      struct nfq_data *nfa, void *data)
//{
//	printf("%d", ++i);
//	struct nfqnl_msg_packet_hdr *ph;
//	ph = nfq_get_msg_packet_hdr(nfa);
//	int id = ntohl(ph->packet_id);
//	return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
//}
//
//int main(int argc, char **argv)
//{
//	struct nfq_handle *h;
//	struct nfq_q_handle *qh;
//	int fd;
//	int rv;
//	char buf[4096] __attribute__ ((aligned));
//
//	printf("opening library handle\n");
//	h = nfq_open();
//	if (!h) {
//		fprintf(stderr, "error during nfq_open()\n");
//		exit(1);
//	}
//
//	printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
//	if (nfq_unbind_pf(h, AF_INET) < 0) {
//		fprintf(stderr, "error during nfq_unbind_pf()\n");
//		exit(1);
//	}
//
//	printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
//	if (nfq_bind_pf(h, AF_INET) < 0) {
//		fprintf(stderr, "error during nfq_bind_pf()\n");
//		exit(1);
//	}
//
//	printf("binding this socket to queue '0'\n");
//	qh = nfq_create_queue(h,  0, &cb, NULL);
//	if (!qh) {
//		fprintf(stderr, "error during nfq_create_queue()\n");
//		exit(1);
//	}
//
//	printf("setting copy_packet mode\n");
//	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
//		fprintf(stderr, "can't set packet_copy mode\n");
//		exit(1);
//	}
//
//	fd = nfq_fd(h);
//
//	while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
//		printf("pkt received\n");
//		nfq_handle_packet(h, buf, rv);
//	}
//
//	printf("unbinding from queue 0\n");
//	nfq_destroy_queue(qh);
//
//#ifdef INSANE
//	/* normally, applications SHOULD NOT issue this command, since
//	 * it detaches other programs/sockets from AF_INET, too ! */
//	printf("unbinding from AF_INET\n");
//	nfq_unbind_pf(h, AF_INET);
//#endif
//
//	printf("closing library handle\n");
//	nfq_close(h);
//
//	exit(0);
//}

#include "NetfilterWrapper.h"
#include "BlockingQueue.h"
#include "HttpPacketHandler.h"
#include "Connector.h"
#include "AgentTypes.h"
#include <memory>
#include <cstring>

AgentArgs* parseArgs(int argc, char **argv)
{
	argc -=1;
	if(argc > 6 || argc%2 != 0)
		throw std::runtime_error("Invalid argument!");

	AgentArgs* args = new AgentArgs();

	for(int i = 1; i < argc; i+=2)
	{
		std::string arg = argv[i];

		if(arg == "-i")
		{
			if(inet_aton(argv[i+1], &args->i) == 0)
			{
				throw std::runtime_error(strerror(errno));
			}
		}
		else if(arg == "-p")
		{
			char* aa;
			args->p = strtol(argv[i+1], &aa, 0);
			if(aa[0] != '\0')
			{
				throw std::runtime_error("Invalid argument value!");
			}
		}
		else if(arg == "-q")
		{
			char* aa;
			args->q = strtol(argv[i+1], &aa, 0);
			if(aa[0] != '\0')
			{
				throw std::runtime_error("Invalid argument value!");
			}
		}
		else
		{
			throw std::runtime_error("Invalid argument!");
		}
	}

	return args;
}

void usage()
{
	std::cout << "Valid usage: [-i] | [-p] | [-q]" << std::endl;
	std::cout << "\t-i - server IP address (default 127.0.0.1)" << std::endl;
	std::cout << "\t-p - server port (default 5000)" << std::endl;
	std::cout << "\t-i - queue number to be used  (default 0)" << std::endl;
}

int main(int argc, char **argv) {
	std::shared_ptr<BlockingQueue<std::shared_ptr<Packet>>> tcpPacketsQueue(new BlockingQueue<std::shared_ptr<Packet>>);

	pthread_t wrapperThread;
	NetfilterWrapper* wrapper = 0;

	pthread_t handlerThread;
	HttpPacketHandler* handler = 0;

	pthread_t connectorThread;
	Connector* connector = 0;

	AgentArgs* args = 0;

	try
	{
		args = parseArgs(argc, argv);

	} catch (const std::runtime_error& e) {
		std::clog << "Error: " << e.what() << std::endl;
		usage();
		delete args;
		exit(0);
	}

	try {

		handler = new HttpPacketHandler(tcpPacketsQueue);
		handlerThread = handler->start();

		wrapper = new NetfilterWrapper(tcpPacketsQueue, args->q);
		wrapperThread = wrapper->start();

		connector = new Connector(handler, args->i, args->p);
		connectorThread = connector->start();

		std::cout << "Agent ready." << std::endl;

		pthread_join(handlerThread, NULL);
		pthread_join(wrapperThread, NULL);
		pthread_join(connectorThread, NULL);
	} catch (const std::runtime_error& e) {
		std::clog << "Error: " << e.what() << std::endl;
	}

	delete args;
	delete handler;
	delete wrapper;
	delete connector;

	return 0;
}
