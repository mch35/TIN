#include "client.h"
#include <iostream>
using namespace std; 

ClientResponse start(time_t when) {
	cout << "starting!" << endl; 
	return OK; 
}

ClientResponse stop(time_t when) {
	cout << "stopping!" << endl; 
	return OK; 
}

ClientResponse get_data() {
	cout << "data!" << endl; 
	return OK; 
}

vector<request_data> get_data_records() {
	vector<request_data> v; 
	
	request_data r1, r2, r3; 
	
	r1.time = time(NULL); 
	r1.method = PUT; 
	r1.receiver_ip.s_addr = inet_addr("52.117.171.205"); 
	v.push_back(r1); 
	
	r2.time = time(NULL); 
	r2.method = POST;
	r2.receiver_ip.s_addr = inet_addr("222.161.171.232"); 
	v.push_back(r2); 

	r3.time = time(NULL); 
	r3.method = TRACE;  
	r3.receiver_ip.s_addr = inet_addr("151.143.99.201"); 
	v.push_back(r3); 
	
	
	return v; 
}
