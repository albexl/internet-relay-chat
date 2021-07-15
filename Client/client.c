#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "command.h"
#include "utils.h"

// terminal
int terin = STDIN_FILENO, terout = STDOUT_FILENO;

// to read
char buffer[MAX_SIZE_BUFF];
char parsed[MAX_SIZE_BUFF];
char nick[MAX_SIZE_BUFF];

// select
fd_set read_fd, ready_fd;
int MAX_FD = -1;

//utilities
int fds = -1;

int main(int argc, char **argv){
	

	//int fds = InitConnection("127.0.0.1",8080);
	

	FD_SET(terin, &read_fd);
	//FD_SET(fds, &read_fd);

	if(terin > MAX_FD)
		MAX_FD = terin;

	while(1)
	{
		// schedule
		ready_fd = read_fd;
		select(MAX_FD + 1, &ready_fd, NULL, NULL, NULL);

		if(FD_ISSET(terin, &ready_fd))
		{
			CLR(buffer);
			int rd = read(terin, buffer, 1024);
			// al berro // client req
			if(buffer[0] != '/'){
				if(fds == -1)
					continue;
				Send_With_Protocol(buffer,rd,fds);
				Write_To_File("Chat.txt",buffer);
			}
			else{
				CLR(parsed);
				int pos = 0;
				for(int i = 0; i < rd - 1; i++){
					parsed[pos++] = buffer[i];
					if(parsed[pos - 1] == ' ')
						break;
				}

				if(!strcmp(parsed,"/connect ")){
					CLR(parsed);
					if(fds != -1)
						continue;
					int idx = 0;
					for( ;pos < rd - 1; pos++){
						if(buffer[pos] == ' ')
							break;
						parsed[idx++] = buffer[pos];
					}
					pos++;

					int port = 0;
					for(int i = pos; i < rd - 1; i++){
						if(!(buffer[i] >= '0' && buffer[i] <= '9'))
							break;
						port = port * 10 + (buffer[i] - '0');
					}

					fds = InitConnection(parsed,port);
					FD_SET(fds, &read_fd);
					if(fds > MAX_FD)
						MAX_FD = fds;
					Write_To_File("Log.txt",buffer);
				}
				else{
					if(fds == -1)
						continue;
					Send_With_Protocol(buffer,rd,fds);
					Write_To_File("Log.txt",buffer);
					if(!strcmp(parsed,"/exit")){
						fds = -1;
						FD_CLR(fds,&read_fd);
						if(FD_ISSET(fds, &ready_fd))
							FD_CLR(fds, &ready_fd);
						write(terout,"you have been disconnected\n",27);
						break;
					}
				}

				if(!strcmp(parsed,"/private ")){
					CLR(parsed);
					int idx = 0;
					for(int i = pos; i < rd - 1; i++){
						parsed[idx++] = buffer[i];
					}
					Write_To_File("Nick.txt",buffer);
				}
			
			}
			
		}

		if(fds == -1)
			continue;
		if(FD_ISSET(fds, &ready_fd))
		{

			CLR(buffer);
			
			int len = Read_With_Protocol(buffer,fds);
			// server req
			write(terout,buffer,len);

			if(!strcmp(buffer,"you have been disconnected\n")){
				FD_CLR(fds,&read_fd);

				if(FD_ISSET(fds, &ready_fd))
					FD_CLR(fds, &ready_fd);
				fds = -1;
			}
			
		}


	}
	return 0;
}
