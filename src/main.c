//
// Created by vladkanash on 16.9.16.
//

/* A simple server in the internet domain using TCP
   The port number is passed as an argument

   Usage:
        Start this, then do 'nc 127.0.0.1 [port_number]'
        and send ur messages*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>

// WINDOWS LIBS 
#include <winsock2.h>
#include <process.h>

#include "server/server.h"

int winsock_version(){
	  return (int)0x0202; // equal to windows macros MAKEWORD(2,2);
}

int initialize_windows_sockets(int (*version)()){
	  WSADATA wsa;

	  printf("Initialising Winsock...\n");
    if (WSAStartup(version(), &wsa) != 0) {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
}

void initialize_sokets(){
    initialize_windows_sockets(winsock_version);
}

int main(int argc, char *argv[]){
	  int portno;
    struct sockaddr_in serv_addr;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    
    initialize_sokets();
     
    printf("Initialised.\n");

    portno = atoi(argv[1]);

    //bzero((char *) &serv_addr, sizeof(serv_addr));
    memset((char *) &serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t) portno);

	//printf("My process ID : %d\n", getpid());
    printf("My process ID : %d\n", _getpid());
    printf("Listening to port : %d\n", portno);

    run_server(&serv_addr);
}
