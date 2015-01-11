/*
 * BlockingQueue.h
 *
 *  Created on: Jan 10, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_BLOCKINGQUEUE_H_
#define SRC_AGENT_BLOCKINGQUEUE_H_
#include <deque>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <pthread.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <iostream>

class BlockingQueue {
	private:
		std::deque<nfq_data*> queue;
		pthread_mutex_t access;
		pthread_cond_t empty;
	public:
		BlockingQueue();
		virtual ~BlockingQueue();

		void add(nfq_data* data);
		nfq_data* get();
};

#endif /* SRC_AGENT_BLOCKINGQUEUE_H_ */
