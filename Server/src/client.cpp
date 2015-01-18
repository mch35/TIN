#include "client.h"

int main( int argc, char *argv[] )
{
	if (argc != 2) {
		cout << "Incorrect parameter. Pass server IP address. Example: ./client 127.0.0.1" << endl;
		return 1; 
	}

	int sockfd = 0, num = 0;
	struct sockaddr_in serv_addr;
	
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		cout<< "Error: could not create socket" << endl;
		return 1;
	}
 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000);
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
 
	if ( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ) {
		cout<< "Error: connection failed" << endl;
		return 1;
	}
	
	unsigned char buffer[COMMAND_LENGTH], response[1];  
	
	while(1) {
		num = recv(sockfd, buffer, COMMAND_LENGTH, 0);
		if ( num <= 0 ) {
			cout<< "Connection closed or error" << endl;
			break;
		}

		command c = deserialize_command(buffer); 
	
		switch (c.type) {
			case START:
				response[0] = (unsigned char)start(c.time);
				break; 
			
			case STOP:
				response[0] = (unsigned char)stop(c.time); 
				break; 
				
			case GET_DATA:
				response[0] = (unsigned char)get_data(); 
				break; 
		}

		if ( (send(sockfd, response, 1, 0)) == -1 ) {
			cout<< "Failure sending message" << endl;
			close(sockfd);
			exit(1);
		}
		
		if (c.type == GET_DATA) {
			vector<request_data> data = get_data_records(); 
			
			unsigned char* s_size = serialize_int(data.size()); 
			if ( (send(sockfd, s_size, INT_LENGTH, 0)) == -1 ) {
				cout<< "Failure sending message" << endl;
				close(sockfd);
				exit(1);
			}
			delete(s_size); 
			
			unsigned char* s_data; 			
			for (vector<request_data>::iterator it = data.begin(); it != data.end(); ++it) {
				s_data = serialize_request(*it); 
				if ( (send(sockfd, s_data, R_D_LENGTH, 0)) == -1 ) {
					cout<< "Failure sending message" << endl;
					close(sockfd);
					exit(1);
				}
				delete(s_data); 
			}
		}
	}
	
	close(sockfd); 
	
	return 0;
}


