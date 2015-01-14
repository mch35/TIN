/*
 * NetfilterWrapper.h
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_NETFILTERWRAPPER_H_
#define SRC_AGENT_NETFILTERWRAPPER_H_

#include <netinet/in.h>
#include <linux/netfilter.h>
#include <libnfnetlink/libnfnetlink.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <pthread.h>

/**
 * Wrapper biblioteki netfilter. Obsługuje kolejkę biblioteki
 * i wrzuca pakiety tcp do przekazanej kolejki w konstruktorze.
 *
 */
class NetfilterWrapper {
	private:
		struct nfq_handle *h;
		struct nfq_q_handle *qh;
		int fd;
		int rv;
		int queueNumber;
		char buf[4096] __attribute__ ((aligned));

		pthread_t worker;

		void* copy();
		static void* copyHelper(void* ctx);
	public:
		NetfilterWrapper(int queueNumber);
		virtual ~NetfilterWrapper();

		/**
		 * Rozpoczyna przekazywanie pakietow do kolejki.
		 *
		 */
		pthread_t start();

		/**
		 * Zatrzymuje przekazywanie pakietow do kolejki.
		 */
		void stop();
};

int callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data);

#endif /* SRC_AGENT_NETFILTERWRAPPER_H_ */
