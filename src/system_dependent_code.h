#ifndef SYSTEM_DEPENDENT_CODE_H
#define SYSTEM_DEPENDENT_CODE_H

#ifdef _WIN32
	#include <winsock2.h>
	#include <process.h>
#else
	#include <sys/socket.h>
	#include <sys/sendfile.h>
	#include <netinet/in.h>
	#include <unistd.h>
    #include <signal.h>
#endif

#include <stdio.h>
#include "server/server.h"

int get_process_id();
int initialize_socket();
int send_data(int socket, const char *buffer, int length, int flags);
int close_socket(int socket);
int quit_socket();
void init_stop_handler();

#endif //SYSTEM_DEPENDENT_CODE_H