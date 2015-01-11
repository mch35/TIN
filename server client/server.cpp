#include "server.h"


// shared variables: 
int client_id_counter = 0;
vector<client_data> clients; 

int listenfd = 0; 
struct sockaddr_in serv_addr;  

pthread_mutex_t mutex; 
pthread_t listener_thread; 

void list_clients() ; 
void send_to_client(int, command); 
bool init(); 
void* listener(void*); 
void ui(); 
void cleanup(); 
void safe_erase(int); 

int main()
{ 
	if (!init()) return 1; 
	
	if (pthread_mutex_init(&mutex, NULL) != 0) {
		cout << "ERROR #5" << endl; 
		return 1; 
	}
	
	int err; 
	if ( (err = pthread_create(&listener_thread, NULL, &listener, (void *) NULL) ) != 0 ) {
		cout << "Error: " << strerror(err) << endl; 
		return 1; 
	}
	
	ui(); 
	
	cleanup(); 
	
	return 0;
}

void* listener(void*) {
	int sockfd; 
	while(1) {
		sockfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		
		client_data cd; 
		
		pthread_mutex_lock(&mutex); 
		
		cd.sockfd = sockfd; 
		cd.id = ++client_id_counter; 
		clients.push_back(cd); 

		pthread_mutex_unlock(&mutex); 
	}
}

time_t convert_time(string s) { return time(NULL); }
void ui() {
	string type, time; 
	int client_id;
	command c; 
	while(1) {
		cout << "% "; 
		type = "", time = "", client_id = 0; 
		cin >> type; 
		
		if (type == "start") {
			c.type = START; 
			cin >> client_id >> time; 
			c.time = convert_time(time); // TODO 
		} else if (type == "stop") {
			c.type = STOP; 
			cin >> client_id >> time; 
			c.time = convert_time(time); // TODO 
		} else if (type == "get_data") {
			cin >> client_id; 
			c.type = GET_DATA; 
		} else if (type == "list_clients") {
			list_clients(); 
			continue;  
		} else if (type == "quit") {
			return; 
		} else {
			cout << "Can't recognize command. Try again." << endl; 
			continue; 
		}
		
		send_to_client(client_id, c); 
	}
}

bool init() {
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
  
	memset(&serv_addr, '0', sizeof(serv_addr));
      
	serv_addr.sin_family = AF_INET;    
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(5000);    
 
	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  
	if(listen(listenfd, 10) == -1){
      cout << "Failed to listen" << endl; 
      return false; 
	}
	
	return true; 
}

void list_clients() {
	if (clients.empty()) {
		cout << "No clients connected" << endl; 
		return; 
	} 
	
	cout << "Client list: " << endl;
	struct sockaddr_in sa;
	unsigned int sa_size = sizeof(sa); 
	for (vector<client_data>::iterator it = clients.begin(); it != clients.end(); ++it) {
		getsockname( it->sockfd, (struct sockaddr*)&sa, &sa_size ); 
		cout << "id: " << it->id << ", IP: " << inet_ntoa(sa.sin_addr) << endl; 
	}
}

void send_to_client(int client_id, command c) {
	pthread_mutex_lock(&mutex); 
	
	vector<client_data>::iterator it; 
	for (it = clients.begin(); it != clients.end(); ++it) {
		if (it->id == client_id) {
			break; 
		}
	}
   
   if (it == clients.end()) {
   	cout << "No such client" << endl; 
   	return; 
   }
   
   client_data cd = *it; 
   
   pthread_mutex_unlock(&mutex);
   
  	unsigned char* com = serialize_command(c); 
   int num; 
  	cout << "Sending command to " << it->id << endl;
	if ( (num = send(cd.sockfd, com, COMMAND_LENGTH, 0)) == -1 ) {
		cout << "Failure sending message to " << it->id << endl; 
		cout << "num: " << num << ", errno: " << errno << endl; 
		safe_erase(cd.id); 
		return; 
	}

	unsigned char buff[1]; 
	if ((num = recv(cd.sockfd, buff, 1, 0)) == -1) {
		cout << "Error while receiving response from " << cd.id << endl; 
		safe_erase(cd.id); 
		return; 
	} else if (num == 0) {
		cout << "Connection to " << cd.id << " is closed" << endl;        
		safe_erase(cd.id); 
		return; 
	}
	
	ClientResponse response = (ClientResponse)buff[0]; 
	switch (response) {
		case OK:
			cout << "Response from " << cd.id << ": command accepted" << endl; 
		break; 
		
		case ERROR: 
			cout << "Response from " << cd.id << ": error" << endl; 
			return;
	}
	 
	if (c.type == GET_DATA) {
		unsigned char num_buff[INT_LENGTH], rec_buff[R_D_LENGTH];
		if ((num = recv(cd.sockfd, num_buff, INT_LENGTH, 0)) != INT_LENGTH) {
			cout << "Error #6" << endl; 
			safe_erase(cd.id); 
			return; 
		}
		
		int record_num = deserialize_int(num_buff);
		request_data rd; 
		while (record_num--) {
			if ((num = recv(cd.sockfd, rec_buff, R_D_LENGTH, 0)) != R_D_LENGTH) {
				cout << "Error #7" << endl; 
				safe_erase(cd.id); 
				return; 
			}	
			rd = deserialize_request(rec_buff); 
			cout << "ip: " << inet_ntoa(rd.receiver_ip) << ", method: " << rd.method << ", time: " << ctime(&(rd.time))
		}
	}
		
}

void safe_erase(int client_id) {
	pthread_mutex_lock(&mutex); 
	
	vector<client_data>::iterator it; 
	for (it = clients.begin(); it != clients.end(); ++it) {
		if (it->id == client_id) {
			break; 
		}
	}
   
   if (it == clients.end()) {
   	return; 
   }
   
   close(it->sockfd); 
	clients.erase(it); 
	   
   pthread_mutex_unlock(&mutex);
}

void cleanup() {
	pthread_mutex_destroy(&mutex); 
	pthread_cancel(listener_thread); 
	for (vector<client_data>::iterator it = clients.begin(); it != clients.end(); ++it) {
		close(it->sockfd); 
	}
	close(listenfd); 
	// TODO
}




