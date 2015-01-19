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
string get_method_name(HttpMethod); 

int main()
{ 
	if ( signal(SIGINT, cleanup) == SIG_ERR ) {
		perror("signal handling"); 
		return 1; 
	}

	if (!init()) return 1; 
	
	if (pthread_mutex_init(&mutex, NULL) != 0) {
		perror("mutex init"); 
		return 1; 
	}
	
	int err; 
	if ( (err = pthread_create(&listener_thread, NULL, &listener, (void *) NULL) ) != 0 ) {
		perror("create listener thread"); 
		return 1; 
	}
	if ( (err = pthread_create(&web_thread, NULL, &web_listener, (void *) NULL) ) != 0 ) {
		perror("create web listener thread"); 
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
		if (sockfd < 0) {
			perror("client accept"); 
			continue; 
		}
		
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
		perror("mkfifo 1"); 
		cleanup(0); 
		exit(1); 
	}
	if ( mkfifo(fifo2_path, S_IFIFO | 0666 ) == -1 ) {
		perror("mkfifo 2"); 
		cleanup(0); 
		exit(1); 
	}
	
	web_readfd = open(fifo1_path, O_RDWR ); 
	if (web_readfd < 0) {
		perror("open fifo1"); 
		cleanup(0); 
		exit(1); 
	}
	
	web_writefd = open(fifo2_path, O_RDWR ); 
	if (web_writefd < 0) {
		perror("open fifo2"); 
		cleanup(0); 
		exit(1); 
	}
	
	const int MSG_LENGTH = 15; 
	char buffer[1024];
	while(1) {
		if ( read(web_readfd, buffer, MSG_LENGTH) < 1 ) {
			perror("web client read"); 
			cleanup(0); 
			exit(1); 
		}
		
		WebCommand c = (WebCommand)(buffer[0]-48); 
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
				strcpy(t, to_string((int)result).c_str()); 
				write(web_writefd, (void*)t, 1); 
				break; 
				
			case W_GET_DATA:
				char tt[4]; 
				strncpy(tt, buffer+2, 2); 
				tt[3] = '\0';
				client_id = atoi(tt); 
				com.type = (CommandType)c; 
				result = send_to_client(client_id, com); 
				strcpy(tt, to_string((int)result).c_str()); 
				write(web_writefd, (void*)tt, 1); 
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

time_t convert_time(string s) { 
	struct tm tm;
	time_t now = time(0);
	tm = *gmtime(&now);
	strptime(s.c_str(), "%Y-%m-%d %H:%M:%S", &tm); 
	time_t t = mktime(&tm); 
	return mktime(&tm); 
}
void ui() {
	string type, time, date; 
	int client_id;
	command c; 
	while(1) {
		cout << "% "; 
		type = "", time = "", date = "", client_id = 0; 
		cin >> type; 
		
		if (type == "start") {
			c.type = START; 
			cin >> client_id >> date >> time; 
			time = date + " " + time; 
			c.time = convert_time(time); 
		} else if (type == "stop") {
			c.type = STOP; 
			cin >> client_id >> date >> time; 
			time = date + " " + time; 
			c.time = convert_time(time); 
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
		
		int res = send_to_client(client_id, c); 
		switch (res) {
			case 0: 
				cout << "Client response: OK" << endl; 
				break; 
				
			case 1: 
				cout << "No such client. Try again." << endl; 
				break; 
				
			case 2: 
				cout << "Error communicating with client." << endl; 
				break; 
				
			case 3: 
				cout << "Client response: ERROR" << endl; 
				break; 
		}
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
     	perror("listen"); 
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
		return 2; 
	}

	unsigned char buff[1]; 
	if ((num = recv(cd.sockfd, buff, 1, 0)) == -1) {
		safe_erase(cd.id); 
		return 2; 
	} else if (num == 0) {
		safe_erase(cd.id); 
		return 2; 
	}
	
	ClientResponse response = (ClientResponse)buff[0]; 
	switch (response) {
		case OK:
			if (c.type != GET_DATA) 
				return 0; 
			break; 
		
		case ERROR: 
			return 3; 
	}
	 
	if (c.type == GET_DATA) {
		unsigned char num_buff[INT_LENGTH], rec_buff[R_D_LENGTH];
		if ((num = recv(cd.sockfd, num_buff, INT_LENGTH, 0)) != INT_LENGTH) {
			safe_erase(cd.id); 
			return 2; 
		}
		
		// otwieramy połączenie do zapisu do bazy 
		MYSQL *con = mysql_init(NULL);
		int session_id = -1; 
		struct sockaddr_in sa;
		unsigned int sa_size = sizeof(sa); 
		getsockname( cd.sockfd, (struct sockaddr*)&sa, &sa_size );		
		if (con != NULL) {
			if (mysql_real_connect(con, "localhost", "root", "", "tin", 0, NULL, 0) == NULL) {
				mysql_close(con); 
			}
			else {
				string q = "insert into sessions values(NULL, "; 
				q += to_string(cd.id); 
				q += ", '"; 
				q += (string)inet_ntoa(sa.sin_addr); 
				q += "')"; 
				cerr << q << endl; 
				if  (mysql_query(con, q.c_str()) ) {
					cerr << "error 1" << endl; 
					mysql_close(con); 
				} else {
					session_id = mysql_insert_id(con); 
				}
			}
		}	
		
		int record_num = deserialize_int(num_buff);
		request_data rd; 
		while (record_num--) {
			if ((num = recv(cd.sockfd, rec_buff, R_D_LENGTH, 0)) != R_D_LENGTH) {
				safe_erase(cd.id); 
				return 2; 
			}	
			rd = deserialize_request(rec_buff);

			if (session_id > 0) {
				char b[20]; 
				memset(b, '\0', 20); 
				strftime( b, 20, "%Y-%m-%d %H:%M:%S", localtime(&(rd.time)) ); 
			
				string qq = "insert into requests values(NULL, '"; 
				qq += (string)inet_ntoa(rd.receiver_ip); 
				qq += "', '"; 
				qq += get_method_name(rd.method); 
				qq += "', '";
				qq += b; 
				qq += "', ";
				qq += to_string(session_id); 
				qq += ", '";
				qq += rd.response;
				qq += "')"; 
				cout << qq << endl; 
				if ( mysql_query(con, qq.c_str()) ) { 
					cerr << "error rp" << endl; 
					mysql_close(con); 
					session_id = -1; 
				}
			}
		}
	}
	
	return 0; 
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

string get_method_name(HttpMethod m) {
	switch(m) {
		case GET:
			return "GET"; 
		case POST:
			return "POST"; 
		case PUT:
			return "PUT";
		case HEAD:
			return "HEAD"; 
		case DELETE:
			return "DELETE"; 
		case TRACE:
			return "TRACE"; 
		case OPTIONS:
			return "OPTIONS"; 
		case CONNECT:
			return "CONNECT"; 
		case PATCH:
			return "PATCH"; 
		case RESPONSE:
			return "RESPONSE";
	}
}



