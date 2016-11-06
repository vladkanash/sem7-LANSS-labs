#ifndef SYSTEM_DEPENDENT_CODE_H
#define SYSTEM_DEPENDENT_CODE_H

#ifdef _WIN32
	#include <winsock2.h>
	#include <process.h>
#else
	#include <sys/socket.h>
	#include <sys/sendfile.h>
	#include <netinet/in.h>
#endif

#include <stdio.h>

int get_process_id();
int initialize_socket();
int send_data(int soket, const char *buffer, int length, int flags);
int close_socket(int socket);
int quit_socket();

#endif //SYSTEM_DEPENDENT_CODE_H