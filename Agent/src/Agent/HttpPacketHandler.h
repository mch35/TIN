/*
 * HttpPacketHandler.h
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_HTTPPACKETHANDLER_H_
#define SRC_AGENT_HTTPPACKETHANDLER_H_
/*#include <netinet/in.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <pthread.h>*/
#include <vector>
#include "communication.h"
#include <atomic>
#include <memory>
#include "BlockingQueue.h"
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include "packet.h"

/**
 * Filtruje pakiety HTTP na podstawie aktualnie ustawionych granic czasowych.
 * Zakłada się, że potrzebne dane na temat pakietu HTTP zawierają się w jednym pakiecie TCP.
 * Dane na temat pakietu HTTP są zapisywane jeżeli startTime <= czas otrzymania pakietu <= stopTime.
 * Domyślnie startTime oraz stopTime = 0 czyli żaden pakiet nie zostanie zapisany.
 *
 */
class HttpPacketHandler {
	private:
		std::shared_ptr<BlockingQueue<std::shared_ptr<Packet>>> tcpPacketsQueue;

		time_t startTime;
		time_t stopTime;

		vector<request_data> httpPackets;

		pthread_t handlerThread;
		pthread_mutex_t timeAccess;

		std::atomic<bool> dataReady;

		void* handleTcpPackets();
		static void* handleTcpPacketsHelper(void*);
		bool isInTime(time_t tcpPacket);
	public:
		HttpPacketHandler(std::shared_ptr<BlockingQueue<std::shared_ptr<Packet>>>tcpQueue);
		virtual ~HttpPacketHandler();

		/**
		 * Zwraca dane zapisane do danej chwili, dla aktualnie zdefiniowanych granic czasowych
		 *
		 */
		vector<std::shared_ptr<request_data>>* getData();

		time_t getStartTime() {
			pthread_mutex_lock(&timeAccess);
			time_t copy = startTime;
			pthread_mutex_unlock(&timeAccess);

			return copy;
		}

		void setStartTime(time_t startTime, time_t stopTime) {
			if(startTime > stopTime)
			{
				throw logic_error("Start time have to be less than stop time!");
			}

			// blokuje zeby wszystkie pakiety od teraz zalapaly sie do nowego pomiaru
			pthread_mutex_lock(&timeAccess);
			if(startTime < time(0))
			{
				pthread_mutex_unlock(&timeAccess);
				throw logic_error("Start time have to be greater than current time!");
			}

			this->startTime = startTime;
			this->stopTime = stopTime;
			this->httpPackets.clear();
			this->dataReady.store(false);

			std::cout << "Starting new filtering. Start: " << ctime(&startTime) << " Stop: " << ctime(&stopTime) << std::endl;
			pthread_mutex_unlock(&timeAccess);
		}

		time_t getStopTime() {
			pthread_mutex_lock(&timeAccess);
			time_t copy = stopTime;
			pthread_mutex_unlock(&timeAccess);

			return copy;
		}

		void setStopTime(time_t stopTime) {
			pthread_mutex_lock(&timeAccess);
			this->stopTime = stopTime;
			this->httpPackets.clear();
			this->dataReady.store(false);
			pthread_mutex_unlock(&timeAccess);
		}

		/**
		 * Zwraca prawdę jeżeli zapisano wszystkie dane dla aktualnie zdefiniowanych granic czasowych
		 *
		 */
		const bool isDataReady() const {
			return dataReady.load();
		}
};

#endif /* SRC_AGENT_HTTPPACKETHANDLER_H_ */
