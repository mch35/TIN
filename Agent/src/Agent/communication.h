#ifndef SRC_AGENT_COMMUNICATION_H_
#define SRC_AGENT_COMMUNICATION_H_

#include <time.h>
#include <string>
#include <arpa/inet.h>
using namespace std;
// Command types:
enum CommandType : unsigned char {
START = 1,
STOP = 2,
GET_DATA = 3
};
// Possible client response:
enum ClientResponse : unsigned char {
OK = 1,
ERROR = 2
};
// HTTP Methods:
enum HttpMethod : unsigned char {
GET = 1,
POST = 2,
PUT = 3,
HEAD = 4,
DELETE = 5,
TRACE = 6,
OPTIONS = 7,
CONNECT = 8,
PATCH = 9
};
// Command to be sent by server.
struct command {
CommandType type;
time_t time;
};
// length of serialized command structure
const int COMMAND_LENGTH = 5;
const int INT_LENGTH = 4;
const int R_D_LENGTH = 9;
// Single HTTP request data - sent by client to server
struct request_data {
time_t time;
HttpMethod method;
in_addr receiver_ip;
};
unsigned char* serialize_command(command);
command deserialize_command(unsigned char*);
unsigned char* serialize_request(request_data);
request_data deserialize_request(unsigned char*);
unsigned char* serialize_int(unsigned int);
unsigned int deserialize_int(unsigned char*); 

#endif
