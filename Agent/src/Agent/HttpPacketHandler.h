/*
 * HttpPacketHandler.h
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_HTTPPACKETHANDLER_H_
#define SRC_AGENT_HTTPPACKETHANDLER_H_

#include "BlockingQueue.h"
#include "Measurement.h"

class HttpPacketHandler {
	private:
		BlockingQueue* queue;
	public:
		HttpPacketHandler(BlockingQueue* queue);
		virtual ~HttpPacketHandler();

		void addMeasurement(Measurement& measurement);
};

#endif /* SRC_AGENT_HTTPPACKETHANDLER_H_ */
