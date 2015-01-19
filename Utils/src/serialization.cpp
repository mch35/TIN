#include "communication.h"
#include <cstdio>

	
unsigned char* serialize_int(unsigned int t) {
	unsigned char* c = new unsigned char [4]; 
	c[0] = t / 256 / 256 / 256 % 256;
	c[1] = t / 256 / 256 % 256;
	c[2] = t / 256 % 256;
	c[3] = t % 256;
	return c; 
}

unsigned int deserialize_int(unsigned char* c) {
	unsigned int t = 0; 
	t += c[0] * 256 * 256 * 256;
	t += c[1] * 256 * 256;
	t += c[2] * 256;
	t += c[3];
	return t; 
}
	
unsigned char* serialize_command(command comm) {
	unsigned char* c = new unsigned char [5]; 
	c[0] = comm.type;
	unsigned char* c2 = serialize_int(static_cast<unsigned int> (comm.time));
	for (int i = 0; i < 4; ++i) c[1+i] = c2[i]; 
	delete c2;
	
	return c; 
}

command deserialize_command(unsigned char* c) {
	command comm;
	comm.type = (CommandType)c[0]; 
	comm.time = static_cast<time_t> (deserialize_int(c+1)); 
	return comm; 
}
 
unsigned char* serialize_request(request_data req) {
	unsigned char* c = new unsigned char [R_D_LENGTH]; 
	
	unsigned char* c2 = serialize_int(static_cast<unsigned int> (req.time));
	for (int i = 0; i < 4; ++i) c[i] = c2[i]; 
	delete c2; 
	
	c[4] = req.method; 
	
	c2 = serialize_int(req.receiver_ip.s_addr); 
	for (int i = 0; i < 4; ++i) c[i+5] = c2[i]; 
	delete c2;
	
	for (int i = 0; i < 4; ++i) c[i+9] = req.response[i];
	
	return c; 
}
 
request_data deserialize_request(unsigned char* c) {
	request_data req; 
	req.time = static_cast<unsigned int> (deserialize_int(c)); 
	req.method = (HttpMethod)c[4]; 
	req.receiver_ip.s_addr = deserialize_int(c+5); 
	for (int i = 0; i < 4; ++i) req.response[i] = c[9+i];
	return req; 
}

 
