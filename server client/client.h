#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include "communication.h"
using namespace std; 

ClientResponse start(time_t when); 
ClientResponse stop(time_t when); 
ClientResponse get_data(); 
vector<request_data> get_data_records(); 

