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



int InitConnection(char *ip,int port);

void PublishSMS(char *p,int len);

int Read_With_Protocol(char *p,int len);

void Send_With_Protocol(char *p,int len,int fd);

void Write_To_File(char *name_file,char *line);

#endif // CHAT_COMMAND_H