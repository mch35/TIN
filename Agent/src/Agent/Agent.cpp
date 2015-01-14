#include "NetfilterWrapper.h"
#include "BlockingQueue.h"
#include "HttpPacketHandler.h"
#include "Connector.h"
#include "AgentTypes.h"
#include <memory>
#include <cstring>

AgentArgs* parseArgs(int argc, char **argv)
{
	AgentArgs* args = new AgentArgs();

	if(argc == 2)
	{
		std::string arg = argv[1];
		if(arg == "-h")
		{
			args->h = true;
			return args;
		}
	}

	argc -= 1;

	if(argc > 6 || argc%2 != 0)
		throw std::runtime_error("Invalid argument!");


	for(int i = 1; i < argc; i+=2)
	{
		std::string arg = argv[i];

		if(arg == "-i")
		{
			if(inet_aton(argv[i+1], &args->i) == 0)
			{
				throw std::runtime_error(strerror(errno));
			}
		}
		else if(arg == "-p")
		{
			char* aa;
			args->p = strtol(argv[i+1], &aa, 0);
			if(aa[0] != '\0')
			{
				throw std::runtime_error("Invalid argument value!");
			}
		}
		else if(arg == "-q")
		{
			char* aa;
			args->q = strtol(argv[i+1], &aa, 0);
			if(aa[0] != '\0')
			{
				throw std::runtime_error("Invalid argument value!");
			}
		}
		else
		{
			throw std::runtime_error("Invalid argument!");
		}
	}

	return args;
}

void usage()
{
	std::cout << "Valid usage: Agent [-i] | [-p] | [-q]" << std::endl;
	std::cout << "\t-h - help" << std::endl;
	std::cout << "\t-i - server IP address (default 127.0.0.1)" << std::endl;
	std::cout << "\t-p - server port (default 5000)" << std::endl;
	std::cout << "\t-i - queue number to be used  (default 0)" << std::endl;
}

int main(int argc, char **argv) {
	std::shared_ptr<BlockingQueue<std::shared_ptr<Packet>>> tcpPacketsQueue(new BlockingQueue<std::shared_ptr<Packet>>);

	pthread_t wrapperThread;
	NetfilterWrapper* wrapper = 0;

	pthread_t handlerThread;
	HttpPacketHandler* handler = 0;

	pthread_t connectorThread;
	Connector* connector = 0;

	AgentArgs* args = 0;

	try
	{
		args = parseArgs(argc, argv);
		if(args->h)
		{
			usage();
			exit(0);
		}
	} catch (const std::runtime_error& e) {
		std::clog << "Error: " << e.what() << std::endl;
		usage();
		delete args;
		exit(0);
	}

	try {

		handler = new HttpPacketHandler(tcpPacketsQueue);
		handlerThread = handler->start();

		wrapper = new NetfilterWrapper(tcpPacketsQueue, args->q);
		wrapperThread = wrapper->start();

		connector = new Connector(handler, args->i, args->p);
		connectorThread = connector->start();

		std::cout << "Agent ready." << std::endl;

		pthread_join(connectorThread, NULL);
	} catch (const std::runtime_error& e) {
		std::clog << "Error: " << e.what() << std::endl;
	}

	delete args;
	delete handler;
	delete wrapper;
	delete connector;

	return 0;
}
