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

//utility
int cur_port = 0;
int fds = -1;

// terminal
int terin = STDIN_FILENO, terout = STDOUT_FILENO;

// to read
char buffer[MAX_SIZE_BUFF];
char parsed[MAX_SIZE_BUFF];
char nick[MAX_SIZE_BUFF];

// select
fd_set read_fd, ready_fd;
int MAX_FD = -1;

int main(int argc, char **argv){
	

	FD_SET(terin, &read_fd);
	//FD_SET(fds, &read_fd);

	if(terin > MAX_FD)
		MAX_FD = terin;
	//if(fds > MAX_FD)
	//	MAX_FD = fds;
	

	while(1)
	{
		// schedule
		ready_fd = read_fd;
		select(MAX_FD + 1, &ready_fd, NULL, NULL, NULL);

		if(FD_ISSET(terin, &ready_fd))
		{

			CLR(buffer);
			int rd = read(terin, buffer, 1024);
			CLR(parsed);
			int pos = 0;

			for(int i = 0; i < rd - 1; i++){
				parsed[pos++] = buffer[i];
				if(parsed[pos - 1] == ' ')
					break;
			}

			if(!strcmp(parsed,"/start ")){
				CLR(parsed);
				int port = 0;
				for(int i = pos; i < rd - 1; i++){
					if(!(buffer[i] >= '0' && buffer[i] <= '9'))
						break;
					port = port * 10 + (buffer[i] - '0');
				}

				fds = InitConnection(port);

				FD_SET(fds, &read_fd);
				if(fds > MAX_FD)
					MAX_FD = fds;
				cur_port = port;

				Init();
				Write_To_File("Log.txt",buffer);

			}
			else if(fds != -1){
				if(!strcmp(parsed,"/write ")){
					CLR(parsed);
					int idx = 0;
					for(int i = pos; i < rd - 1; i++)
						parsed[idx++] = buffer[i];

					PublicSMS(parsed,fds);

				}
				else if(!strcmp(parsed,"/private ")){
					
					CLR(parsed);
					CLR(nick);

					int idx = 0;

					for(; pos < rd - 1; pos++){
						if(buffer[pos] == ' ')
							break;
						nick[idx++] = buffer[pos];
					}

					pos++;
					idx = 0;
					for(; pos < rd - 1; pos++)
						parsed[idx++] = buffer[pos];

					PrivateSMS(parsed,nick,fds);
				}
				else if(!strcmp(parsed,"/list")){
					List(fds);
					Write_To_File("Log.txt",buffer);
				}
				else if(!strcmp(parsed,"/stop")){
					Stop();
					Write_To_File("Log.txt",buffer);
					fds = -1;
				}
				else if(!strcmp(parsed,"/exit")){
					Exit();
					Write_To_File("Log.txt",buffer);
				}
				else if(!strcmp(parsed,"/kick ")){
					CLR(parsed);
					int idx = 0;
					for(int i = pos; i < rd - 1; i++)
						parsed[idx++] = buffer[i];
					Kick_Nick(parsed);
					Write_To_File("Log.txt",buffer);
				}
				else if(!strcmp(parsed,"/ban ")){
					CLR(parsed);
					int idx = 0;
					for(int i = pos; i < rd - 1; i++)
						parsed[idx++] = buffer[i];
					Ban(parsed);
					Write_To_File("Log.txt",buffer);
				}
				else if(!strcmp(parsed,"/unban ")){
					CLR(parsed);
					int idx = 0;
					for(int i = pos; i < rd - 1; i++)
						parsed[idx++] = buffer[i];
					Unban(parsed);
					Write_To_File("Log.txt",buffer);
				}
				else if(!strcmp(parsed,"/block")){
					List_Blocked();
				}
				else if(!strcmp(parsed,"/block ")){
					int idx = 0;
					CLR(parsed);
					for(int i = pos; i < rd - 1; i++)
						parsed[idx++] = buffer[i];
					Block(parsed);
				}
				else{
					CLR(parsed);
					sprintf(parsed,"syntax error\n");
					write(terout,parsed,strlen(parsed));
				}
			}
			
		}
		if(fds == -1)
			continue;
		if(FD_ISSET(fds, &ready_fd))
		{
			// accept
			int fdclient = Accept(fds);
			if(fdclient >= 0)
			{
				FD_SET(fdclient, &read_fd);
				if(MAX_FD < fdclient)
					MAX_FD = fdclient;
			}
		}

		for (int clientfd = 0; clientfd <= MAX_FD; ++clientfd)
		{
			if(clientfd != fds && clientfd != terin && FD_ISSET(clientfd, &ready_fd))
			{
				// client req;
				CLR(buffer);
				int rd = Read_With_Protocol( buffer, clientfd);
				CLR(parsed);

				if(buffer[0] != '/'){
					PublicSMS(buffer,clientfd);
				}
				else{

					int idx = 0;
					int pos = 0;
					for(int i = 0; i < rd - 1; i++){
						parsed[idx++] = buffer[i];
						pos++;
						if(buffer[i] == ' ')
							break;
					}
					if(!strcmp(parsed,"/nick ")){
						CLR(parsed);
						int idx = 0;
						for(int i = pos; i < rd - 1; i++)
							parsed[idx++] = buffer[i];
						ChangeNick(clientfd,parsed);
						Write_To_File("Log.txt",buffer);
					}
					else if(!strcmp(parsed,"/disconnect")){
						Kick_FD(clientfd);
						Write_To_File("Log.txt",buffer);
					}
					else if(!strcmp(parsed,"/private ")){
						CLR(parsed);
						CLR(nick);

						int idx = 0;

						for(; pos < rd - 1; pos++){
							if(buffer[pos] == ' ')
								break;
							nick[idx++] = buffer[pos];
						}

						pos++;
						idx = 0;
						for(; pos < rd - 1; pos++)
							parsed[idx++] = buffer[pos];

						PrivateSMS(parsed,nick,clientfd);

					}
					else if(!strcmp(parsed,"/exit")){
						Kick_FD(clientfd);
						Write_To_File("Log.txt",buffer);
					}
					else if(!strcmp(parsed,"/list")){
						List(clientfd);
						Write_To_File("Log.txt",buffer);
					}

				}
			}
		}

	}

	return 0;
}
