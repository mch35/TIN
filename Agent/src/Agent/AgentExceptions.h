/*
 * AgentExceptions.h
 *
 *  Created on: Jan 11, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_AGENTEXCEPTIONS_H_
#define SRC_AGENT_AGENTEXCEPTIONS_H_

#include <exception>

class SocketCreationException : public std::exception
{
	virtual const char* what() const throw()
	{
	    return "Could not create socket!";
	}
};

class ConnectionException : public std::exception
{
	virtual const char* what() const throw()
	{
	    return "Connection failed!";
	}
};

#endif /* SRC_AGENT_AGENTEXCEPTIONS_H_ */
