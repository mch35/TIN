/*
 * NetfilterWrapper.cpp
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#include "NetfilterWrapper.h"
#include "BlockingQueue.h"
#include <pthread.h>

BlockingQueue netfilterQueue;

NetfilterWrapper::NetfilterWrapper(int queueNumber) : queueNumber(queueNumber), isStarted(false) {
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
	}

	return 0;
}


void* NetfilterWrapper::copyHelper(void* ctx)
{
	return ((NetfilterWrapper*)ctx)->copy();
}

NetfilterWrapper::~NetfilterWrapper() {
	nfq_destroy_queue(qh);
}

void NetfilterWrapper::start()
{

}

void NetfilterWrapper::stop()
{

}

int callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
	netfilterQueue.add(nfa);
	struct nfqnl_msg_packet_hdr *ph;
	ph = nfq_get_msg_packet_hdr(nfa);
	int id = ntohl(ph->packet_id);

	return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}
