#include "command.h"
#include "utils.h"

extern int terout;
extern int fds;
extern int cur_port;
extern fd_set read_fd;
extern fd_set ready_fd;


int size_clients = 1;
int size_ips = 0;
int size_blocked = 0;

client clients[MAX_SIZE_CLIENTS];

//utilities

char to_send[MAX_SIZE_BUFF];

char *ips[MAX_SIZE_BUFF];
char *blocked[MAX_SIZE_BUFF];


int InitConnection(int port)
{
	struct sockaddr_in s;

	bzero((char *) &s, sizeof(s));
	s.sin_family = AF_INET;
	s.sin_port = htons((unsigned short)port);
	s.sin_addr.s_addr = htonl(INADDR_ANY);

	char BUFF[MAX_SIZE_BUFF];

	int sfd = socket(AF_INET, SOCK_STREAM, 0);
  	if (sfd < 0){
  		CLR(BUFF);
		sprintf(BUFF,"ERROR opening socket\n");
		write(terout, BUFF, strlen(BUFF));
    	return -1;
  	}

	int optval = 1;
	setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(int));

	if(bind(sfd, (struct sockaddr*) &s, sizeof(s)) < 0){
  		CLR(BUFF);
		sprintf(BUFF,"ERROR on binding\n");
		write(terout, BUFF, strlen(BUFF));
    	return -1;
    }
	if (listen(sfd, 10) < 0){
    	CLR(BUFF);
		sprintf(BUFF,"ERROR on listen\n");
		write(terout, BUFF, strlen(BUFF));
    	return -1;
    }

    char b[MAX_SIZE_BUFF] = "--> Waiting for connection...\n";
    write(terout, b, strlen(b));

    return sfd;
}

int Accept(int sfd){

	client * curr_client = &clients[ size_clients++];
	//struct sockaddr_in client;

	char sms[MAX_SIZE_BUFF];
	CLR(sms);

	int client_accepted;
	int size = sizeof(curr_client->addr);

	curr_client->enable = 1;
	
	client_accepted = accept(sfd, (struct sockaddr*)&curr_client->addr, &size);
	
	curr_client->fd = client_accepted;
	

	if (client_accepted < 0){
        
		sprintf(sms,"ERROR on accept\n");
		write(terout, sms, strlen(sms));
        curr_client->enable = 0;
        size_clients--;
        return -1;
    }

    char BUFF[MAX_SIZE_BUFF];
    CLR(BUFF);
    sprintf(BUFF,inet_ntoa(curr_client->addr.sin_addr) );

    for(int i = 0; i < size_ips; i++){
    	if(!strcmp(BUFF,ips[i])){
    		CLR(sms);
    		sprintf(sms,"ERROR on accept\n");
    		write(terout, sms, strlen(sms));
        	curr_client->enable = 0;
        	size_clients--;
        	Send_With_Protocol("you have been disconnected\n",27,client_accepted);
        	return -1;		
    	}
    }

    sprintf(curr_client->nick, "USER%d", size_clients - 1);


    sprintf(sms, "--> Connection established with: %s.\n", inet_ntoa(curr_client->addr.sin_addr));
	write(terout, sms, strlen(sms));


	return client_accepted;
}

void Init(){
	client * curr_client = &clients[ 0 ];
	sprintf(curr_client->nick, "admin");
	curr_client->enable = 1;
	curr_client->fd = fds;
}

void List(int fd){
	for(int i = 1; i <= size_clients; i++){
		if(clients[i].enable){
			char p[MAX_SIZE_BUFF];
			CLR(p);
			client * curr_client = &clients[ i ];
			sprintf(p,"%s: ",curr_client->nick);
			if(fd == fds){
				write(terout, p, strlen(p));
				write(terout, inet_ntoa(curr_client->addr.sin_addr),strlen(inet_ntoa(curr_client->addr.sin_addr)) );
				write(terout,"\n",1);
			}
			else{
				Send_With_Protocol(p,strlen(p),fd);
				Send_With_Protocol(inet_ntoa(curr_client->addr.sin_addr),strlen(inet_ntoa(curr_client->addr.sin_addr)),fd);
				Send_With_Protocol("\n",1,fd);
			}

		}
	}
}

void ChangeNick(int fd,char *p){

	//printf("changing nickname\n");
	//printf("%d\n",fd);
	//printf("%s\n",p );

	client * curr_client;
	for(int i = 0; i <= size_clients; i++){
		if(clients[i].enable && fd == clients[i].fd){
			curr_client = &clients[i];
			break;
		}
	}

	for(int i = 0; i <= size_clients; i++){
		if(clients[i].enable && !strcmp(p,clients[i].nick)){
			Send_With_Protocol("that nick already exists\n",25,fd);
			return;
		}
	}
	sprintf(curr_client->nick,p);
}

void Exit(){
	char sms[MAX_SIZE_BUFF];
	CLR(sms);
	for(int i = 1; i <= size_clients; i++){
		if(clients[i].enable){
			Send_With_Protocol("you have been disconnected\n",27,clients[i].fd);
			client * curr_client = &clients[i];
			sprintf(sms, "--> Connection lost with: %s.\n", inet_ntoa(curr_client->addr.sin_addr));
			write(terout, sms, strlen(sms));
			clients[i].enable = 0;
			FD_CLR(clients[i].fd, &read_fd);
			if(FD_ISSET(clients[i].fd, &ready_fd))
				FD_CLR(clients[i].fd, &ready_fd);
			close(clients[i].fd);
		}
	}
	FD_CLR(fds, &read_fd);
	exit(EXIT_SUCCESS);
}

void Stop(){
	char sms[MAX_SIZE_BUFF];
	CLR(sms);
	for(int i = 1; i <= size_clients; i++){
		if(clients[i].enable){
			Send_With_Protocol("you have been disconnected\n",27,clients[i].fd);
			client * curr_client = &clients[i];
			sprintf(sms, "--> Connection lost with: %s.\n", inet_ntoa(curr_client->addr.sin_addr));
			write(terout, sms, strlen(sms));
			clients[i].enable = 0;
			FD_CLR(clients[i].fd, &read_fd);
			if(FD_ISSET(clients[i].fd, &ready_fd))
				FD_CLR(clients[i].fd, &ready_fd);
			close(clients[i].fd);
		}
	}
	FD_CLR(fds, &read_fd);
	fds = -1;
}

void Kick_Nick(char *nick){
	char sms[MAX_SIZE_BUFF];
	CLR(sms);

	for(int i = 1; i <= size_clients; i++){
		if( clients[i].enable && !strcmp(clients[i].nick,nick) ){
			Send_With_Protocol("you have been disconnected\n",27,clients[i].fd);
			client * curr_client = &clients[i];
			sprintf(sms, "--> Connection lost with: %s.\n", inet_ntoa(curr_client->addr.sin_addr));
			write(terout, sms, strlen(sms));
			
			clients[i].enable = 0;
			FD_CLR(clients[i].fd, &read_fd);
			if(FD_ISSET(clients[i].fd, &ready_fd))
				FD_CLR(clients[i].fd, &ready_fd);
			close(clients[i].fd);
			return;
		}
	}
	write(terout,"that nickname doesn't exists\n",29);
}

void Kick_FD(int fd){
	char sms[MAX_SIZE_BUFF];
	CLR(sms);
	for(int i = 1; i <= size_clients; i++){
		if( clients[i].enable && fd == clients[i].fd ){
			Send_With_Protocol("you have been disconnected\n",27,clients[i].fd);
			client * curr_client = &clients[i];
			sprintf(sms, "--> Connection lost with: %s.\n", inet_ntoa(curr_client->addr.sin_addr));
			write(terout, sms, strlen(sms));
			clients[i].enable = 0;
			
			FD_CLR(clients[i].fd, &read_fd);
			if(FD_ISSET(clients[i].fd, &ready_fd))
				FD_CLR(clients[i].fd, &ready_fd);
			close(clients[i].fd);
			return;
		}
	}
	write(terout,"that nickname doesn't exists\n",29);
}

int Find_In_Blocked(char *p){
	for(int i = 0; i < size_blocked; i++){
		char *pp = &blocked[i];
		if(!strcmp(p,pp))
			return 1;
	}
	return -1;
}

void PublicSMS(char *p,int sender){
	
	Save_Blocked();

	printf("%s\n",p );

	client * curr_client;
	for(int i = 0; i <= size_clients; i++){
		if(clients[i].enable){
			if(sender == clients[i].fd){
				curr_client = &clients[i];
				break;
			}
		}
	}
	
	CLR(to_send);
	int pos = 0;
	for(int i = 0; i < strlen(curr_client->nick); i++)
		to_send[pos++] = curr_client->nick[i];
	to_send[pos++] = ':';
	to_send[pos++] = ' ';

	char extra[MAX_SIZE_BUFF];
	CLR(extra);
	int idx = 0;

	for(int i = 0;i < strlen(p); i++){
		if(p[i] == ' ' || p[i] == '\n'){
			int found = Find_In_Blocked(extra);
			if(found == 1){
				to_send[pos++] = '[';
				to_send[pos++] = 'F';
				to_send[pos++] = 'I';
				to_send[pos++] = 'L';
				to_send[pos++] = 'T';
				to_send[pos++] = 'E';
				to_send[pos++] = 'R';
				to_send[pos++] = 'E';
				to_send[pos++] = 'D';
				to_send[pos++] = ']';
			}
			else{
				for(int j = 0; j < strlen(extra); j++){
					to_send[pos++] = extra[j];
				}
			}
			to_send[pos++] = ' ';
			CLR(extra);
			idx = 0;
		}
		else{
			extra[idx++] = p[i];
		}
	}

	int found = Find_In_Blocked(extra);
	if(found == 1){
		to_send[pos++] = '[';
		to_send[pos++] = 'F';
		to_send[pos++] = 'I';
		to_send[pos++] = 'L';
		to_send[pos++] = 'T';
		to_send[pos++] = 'E';
		to_send[pos++] = 'R';
		to_send[pos++] = 'E';
		to_send[pos++] = 'D';
		to_send[pos++] = ']';
	}
	else{
		for(int j = 0; j < strlen(extra); j++){
			to_send[pos++] = extra[j];
		}
	}

	to_send[pos++] = '\n';
	for(int i = 1; i <= size_clients; i++){
		if(clients[i].enable){
			Send_With_Protocol(to_send,strlen(to_send),clients[i].fd);
		}
	}
	write(terout, to_send, strlen(to_send));
}

void PrivateSMS(char *p, char *nick, int sender){
	int receiver = -1;
	for(int i = 0; i <= size_clients; i++){
		if(clients[i].enable){
			client * curr_client = &clients[i];
			if(!strcmp(nick,curr_client->nick)){
				receiver = curr_client->fd;
				break;
			}
		}
	}
	if(receiver == -1){
		if(sender == fds)
			write(terout,"that nickname doesn't exists\n",29);
		else{
			Send_With_Protocol("that nickname doesn't exists\n",29,sender);
		}
		return;
	}



	client * curr_client;
	for(int i = 0; i <= size_clients; i++){
		if(clients[i].enable){
			if(sender == clients[i].fd){
				curr_client = &clients[i];
				break;
			}
		}
	}
	CLR(to_send);
	int pos = 0;
	for(int i = 0; i < strlen(curr_client->nick); i++)
		to_send[pos++] = curr_client->nick[i];

	to_send[pos++] = ':';
	to_send[pos++] = ' ';
	to_send[pos++] = '[';
	to_send[pos++] = 'P';
	to_send[pos++] = 'R';
	to_send[pos++] = 'I';
	to_send[pos++] = 'V';
	to_send[pos++] = 'A';
	to_send[pos++] = 'T';
	to_send[pos++] = 'E';
	to_send[pos++] = ']';
	to_send[pos++] = ' ';

	for(int i = 0 ; i < strlen(p); i++)
		to_send[pos++] = p[i];

	to_send[pos++] = '\n';

	if(receiver == fds)
		write(terout,to_send,strlen(to_send));
	else
		Send_With_Protocol(to_send,strlen(to_send),receiver);

}

void Ban(char *p){
	ips[size_ips++] = p;
	for(int i = 1; i <= size_clients; i++){
		if(!clients[i].enable)
			continue;
		client * curr_client = &clients[i];
		if(!strcmp(p,inet_ntoa(curr_client->addr.sin_addr) ) ){
			char B[MAX_SIZE_BUFF];
			CLR(B);
			sprintf(B,curr_client->nick);
			Kick_Nick(B);
		}
	}
}

void Unban(char *p){
	for(int i = 0; i < size_ips; i++){
		if(!strcmp( p,ips[i] ) ){
			CLR(ips[i]);
		}
	}
}

void Block(char *p){
	char BUFF[MAX_SIZE_BUFF];
	CLR(BUFF);
	sprintf(BUFF,"%s\n",p);
	Write_To_File("Block.txt",BUFF);
}

void List_Blocked(){
	
	char BUFF[MAX_SIZE_BUFF];
	CLR(BUFF);
	char c = '0';
	int fd = open("Block.txt",O_RDWR | O_APPEND | O_CREAT, 0644);
	int idx = 0;

	while(read(fd, &c, 1)){
		BUFF[idx++] = c;
		if(c == '\n'){
			write(terout,BUFF,strlen(BUFF));
			CLR(BUFF);
			idx = 0;
		}
	}
}

void Save_Blocked(){
	
	char BUFF[MAX_SIZE_BUFF];
	CLR(BUFF);
	char c = '0';
	int fd = open("Block.txt",O_RDWR | O_APPEND | O_CREAT, 0644);
	int idx = 0;

	while(read(fd, &c, 1)){
		if(c == '\n'){

			char *p = &blocked[size_blocked];
			sprintf(p,"%s",BUFF);

			CLR(BUFF);
			idx = 0;
			size_blocked++;
		}
		else{
			BUFF[idx++] = c;	
		}
	}
}

void Send_With_Protocol(char *p,int len,int fd){
	char buff[MAX_SIZE_BUFF];
	CLR(buff);
	sprintf(buff,"%d ",len);
	write(fd,buff,strlen(buff));
	write(fd,p,len);
}

int Read_With_Protocol(char *p,int input){
	int len = 0;
	char c = '0';
	while(c != ' '){
		len = len*10 + (c - '0');
		read(input, &c, 1);
	}
	read(input, p, len);
	return len;
}

void Write_To_File(char *file_name,char *line){

	int fd = open(file_name,O_WRONLY | O_APPEND | O_CREAT, 0644);
	write(fd,line,strlen(line));
	close(fd);

}