/*
 * NetfilterWrapper.h
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_NETFILTERWRAPPER_H_
#define SRC_AGENT_NETFILTERWRAPPER_H_

#include <netinet/in.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <pthread.h>

class NetfilterWrapper {
	private:
		//BlockingQueue queue;
		struct nfq_handle *h;
		struct nfq_q_handle *qh;
		int fd;
		int rv;
		int queueNumber;
		char buf[4096] __attribute__ ((aligned));

		bool isStarted;
		pthread_t worker;

		void openLibrary();
		void unbindHandler();
		void bindHandler();
		void createQueue(nfq_callback *cb);
		void setMode();
		void* copy();
		static void* copyHelper(void* ctx);
	public:
		NetfilterWrapper(int queueNumber);
		virtual ~NetfilterWrapper();

		void start();
		void stop();
};

int callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data);

#endif /* SRC_AGENT_NETFILTERWRAPPER_H_ */
