/*
 * HttpPacketHandler.h
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_HTTPPACKETHANDLER_H_
#define SRC_AGENT_HTTPPACKETHANDLER_H_

#include <pthread.h>
#include <vector>
#include <atomic>
#include <memory>
#include <ctime>
#include <regex>

#include "AgentTypes.h"
#include "BlockingQueue.h"
#include "../../Utils/src/communication.h"

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
		std::atomic<bool> running;

		void* handleTcpPackets();
		static void* handleTcpPacketsHelper(void*);
		bool isInTime(time_t tcpPacket);
	bool tryMatch(const shared_ptr<Packet>& tcpPacket, const regex& txt_regex, cmatch& m);

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

		/**
		 * Ustawia granice czasowe, w których musi mieścić się pakiet http, żeby zostać zapisanym.
		 */
		void setTimeBounds(time_t startTime, time_t stopTime);

		/**
		 * Zwraca prawdę jeżeli zapisano wszystkie dane dla aktualnie zdefiniowanych granic czasowych
		 *
		 */
		const bool isDataReady() const {
			return dataReady.load();
		}

		pthread_t start();
		void stop();
};


#endif /* SRC_AGENT_HTTPPACKETHANDLER_H_ */
