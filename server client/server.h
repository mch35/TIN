#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "communication.h"
#include <iostream>
#include <vector>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <mysql.h>
using namespace std; 

const int MAX_CLIENTS = 50; 

struct client_data {
	int id = 0, sockfd = 0; 
};

