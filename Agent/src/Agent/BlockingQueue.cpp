/*
 * BlockingQueue.cpp
 *
 *  Created on: Jan 10, 2015
 *      Author: root
 */

#include "BlockingQueue.h"

BlockingQueue::BlockingQueue() {
	pthread_mutex_init(&access,0);
	pthread_cond_init(&empty, NULL);
}

BlockingQueue::~BlockingQueue() {
    pthread_mutex_destroy(&access);
    pthread_cond_destroy(&empty);
}

void BlockingQueue::add(nfq_data* data)
{
	pthread_mutex_lock(&access);
	std::cout << "adding" << std::endl;
	queue.push_back(data);
	pthread_cond_signal(&empty);
	pthread_mutex_unlock(&access);
}

nfq_data* BlockingQueue::get()
{
	pthread_mutex_lock(&access);
	while(queue.empty())
		pthread_cond_wait(&empty, &access);

	std::cout << "getting" << std::endl;
	nfq_data* data = queue.front();
	queue.pop_front();
	pthread_mutex_unlock(&access);

	return data;
}

