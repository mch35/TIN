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
#include <memory>

template<typename T> class BlockingQueue {
	private:
		std::deque<T*> queue;
		pthread_mutex_t access;
		pthread_cond_t empty;
	public:
		BlockingQueue();
		virtual ~BlockingQueue();

		void add(T* data);
		T* get();
};

template<typename T>
BlockingQueue<T>::BlockingQueue() {
	pthread_mutex_init(&access,0);
	pthread_cond_init(&empty, NULL);
}

template<typename T>
BlockingQueue<T>::~BlockingQueue() {
    pthread_mutex_destroy(&access);
    pthread_cond_destroy(&empty);
}

template<typename T>
void BlockingQueue<T>::add(T* data)
{
	pthread_mutex_lock(&access);
	queue.push_back(data);
	pthread_cond_signal(&empty);
	pthread_mutex_unlock(&access);
}

template<typename T>
T* BlockingQueue<T>::get()
{
	pthread_mutex_lock(&access);
	while(queue.empty())
		pthread_cond_wait(&empty, &access);

	T* data = queue.front();
	queue.pop_front();
	pthread_mutex_unlock(&access);

	return data;
}


#endif /* SRC_AGENT_BLOCKINGQUEUE_H_ */
