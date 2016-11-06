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

#include "server/server.h"
#include "system_dependent_code.h"

int main(int argc, char *argv[]){
	int portno;
    struct sockaddr_in serv_addr;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
         
    portno = atoi(argv[1]);

    memset((char *) &serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t) portno);

	//printf("My process ID : %d\n", getpid());
    printf("My process ID : %d\n", _getpid());
    printf("Listening to port : %d\n", portno);
	
    run_server(&serv_addr);
}
