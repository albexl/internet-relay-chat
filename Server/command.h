#ifndef CHAT_COMMAND_H
#define CHAT_COMMAND_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include <sys/stat.h>
#include <fcntl.h>


typedef struct 
{
	struct sockaddr_in addr;
	int enable;
	int fd;
	char nick[MAX_SIZE_NICK];

}client;

void Save_Blocked();

int Find_In_Blocked(char *p);

void ChangeNick(int fd,char *p);

void Ban(char *p);

void Unban(char *p);

void Stop();

void Exit();

void List(int fd);

void Init();

void Kick_Nick(char *nick);

void Kick_FD(int fd);

int InitConnection(int port);

int Accept(int sfd);

void Send_With_Protocol(char *p,int len,int fd);

void PublicSMS(char *p,int sender);

void PrivateSMS(char *p, char *nick, int sender);

int Read_With_Protocol(char *p,int len);

void Write_To_File(char *name_file,char *line);

void Block(char *p);

void List_Blocked();


#endif //CHAT_COMMAND_H