#include "server.h"


// shared variables: 
int client_id_counter = 0;
vector<client_data> clients; 

int listenfd = 0, web_readfd, web_writefd;
char fifo1_path[50] = "/tmp/fifo1"; 
char fifo2_path[50] = "/tmp/fifo2";  
struct sockaddr_in serv_addr;  

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_t listener_thread, web_thread; 

void list_clients(); 
int send_to_client(int, command); 
bool init(); 
void* listener(void*); 
void* web_listener(void*); 
void ui(); 
void cleanup(int); 
void safe_erase(int); 

int main()
{ 
	if ( signal(SIGINT, cleanup) == SIG_ERR ) {
		perror("signal handling: "); 
		return 1; 
	}

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
	if ( (err = pthread_create(&web_thread, NULL, &web_listener, (void *) NULL) ) != 0 ) {
		cout << "Error: " << strerror(err) << endl; 
		return 1; 
	}
	
	ui(); 
	
	cleanup(0); 
	
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

void* web_listener(void*) {
	if ( mkfifo(fifo1_path, S_IFIFO | 0666 ) == -1 ) {
		return NULL; 
	}
	if ( mkfifo(fifo2_path, S_IFIFO | 0666 ) == -1 ) {
		return NULL; 
	}
	
	web_readfd = open(fifo1_path, O_RDONLY | O_NONBLOCK ); 
	
	if (web_readfd < 0) {
		return NULL; 
	}
	
	web_writefd = open(fifo2_path, O_WRONLY | O_NONBLOCK ); 
	if (web_writefd < 0) {
		return NULL; 
	}
	
	const int MSG_LENGTH = 15; 
	char buffer[1024];
	while(1) {
		if ( read(web_readfd, buffer, MSG_LENGTH) < 1 ) {
			return NULL; 
		}
		
		WebCommand c = (WebCommand)buffer[0]; 
		int result, client_id; 
		command com; 
		
		switch(c) {
			case W_START:
			case W_STOP:  
				char t[11]; 
				strncpy(t, buffer+2, 2); 
				t[3] = '\0';
				client_id = atoi(t); 
				strncpy(t, buffer+5, 10); 
				t[10] = '\0'; 
				com.type = (CommandType)c; 
				com.time = (time_t)atoi(t); 
				result = send_to_client(client_id, com); 
				memset(t, '\0', 10); 
				t[0] = (char)result;
				write(web_writefd, t, 1);  
				break; 
				
			case W_GET_DATA:
				char tt[4]; 
				strncpy(tt, buffer+2, 2); 
				tt[3] = '\0';
				client_id = atoi(tt); 
				com.type = (CommandType)c; 
				result = send_to_client(client_id, com); 
				memset(tt, '\0', 10); 
				tt[0] = (char)result;
				write(web_writefd, tt, 1); 
				break; 
				
			case W_LIST_CLIENTS:
				pthread_mutex_lock(&mutex); 
				int size = clients.size(); 
				char ttt[20]; 
				memset(ttt, '\0', 20); 
				ttt[0] = (char)(size/10 + 48); 
				ttt[1] = (char)(size%10 + 48); 
				write(web_writefd, ttt, 2); 
				
				struct sockaddr_in sa;
				unsigned int sa_size = sizeof(sa);
				for (vector<client_data>::iterator it = clients.begin(); it != clients.end(); ++it) {
					memset(ttt, '\0', 20); 
					ttt[0] = (char)(it->id/10 + 48); 
					ttt[1] = (char)(it->id%10 + 48); 
					ttt[2] = ' '; 
					getsockname( it->sockfd, (struct sockaddr*)&sa, &sa_size ); 					
					strcpy(ttt+3, inet_ntoa(sa.sin_addr)); 
					write(web_writefd, ttt, 20); 
				}
				pthread_mutex_unlock(&mutex); 
				break; 
		}
		
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

int send_to_client(int client_id, command c) {
	pthread_mutex_lock(&mutex); 
	
	vector<client_data>::iterator it; 
	for (it = clients.begin(); it != clients.end(); ++it) {
		if (it->id == client_id) {
			break; 
		}
	}
   
   if (it == clients.end()) {
   	return 1; 
   }
   
   client_data cd = *it; 
   
   pthread_mutex_unlock(&mutex);
   
  	unsigned char* com = serialize_command(c); 
   int num; 
	if ( (num = send(cd.sockfd, com, COMMAND_LENGTH, 0)) == -1 ) {
		safe_erase(cd.id); 
		return 1; 
	}

	unsigned char buff[1]; 
	if ((num = recv(cd.sockfd, buff, 1, 0)) == -1) {
		safe_erase(cd.id); 
		return 1; 
	} else if (num == 0) {
		safe_erase(cd.id); 
		return 1; 
	}
	
	ClientResponse response = (ClientResponse)buff[0]; 
	switch (response) {
		case OK:
			return 0; 
		break; 
		
		case ERROR: 
			return 1; 
	}
	 
	if (c.type == GET_DATA) {
		unsigned char num_buff[INT_LENGTH], rec_buff[R_D_LENGTH];
		if ((num = recv(cd.sockfd, num_buff, INT_LENGTH, 0)) != INT_LENGTH) {
			safe_erase(cd.id); 
			return 1; 
		}
		
		int record_num = deserialize_int(num_buff);
		request_data rd; 
		while (record_num--) {
			if ((num = recv(cd.sockfd, rec_buff, R_D_LENGTH, 0)) != R_D_LENGTH) {
				safe_erase(cd.id); 
				return 1; 
			}	
			rd = deserialize_request(rec_buff); 
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

void cleanup(int dummy=0) {
	pthread_mutex_destroy(&mutex); 
	pthread_cancel(listener_thread); 
	pthread_cancel(web_thread); 
	for (vector<client_data>::iterator it = clients.begin(); it != clients.end(); ++it) {
		close(it->sockfd); 
	}
	close(listenfd); 
	close(web_readfd); 
	close(web_writefd); 
	unlink(fifo1_path); 
	unlink(fifo2_path); 
}




