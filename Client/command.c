#include "command.h"
#include "utils.h"

extern int terout;

int InitConnection(char *ip,int port){

	int sfd = socket(AF_INET, SOCK_STREAM, 0);
  	if (sfd < 0){
    	printf("ERROR opening socket");
    	return -1;
  	}
	
    int optval = 1;
	setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(int));

	struct sockaddr_in serv_addr;
	
	inet_aton(ip, &serv_addr.sin_addr);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	
	int connection = connect(sfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if(connection < 0){
		printf("ERROR on connect");
		return -1;
	}

    char b[MAX_SIZE_BUFF] = "--> Connection established with server\n";
    write(terout, b, strlen(b));

	return sfd;

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
