//
// Created by vladkanash on 16.9.16.
//
#include <types.h>
#include "client.h"

char uuid[UUID_LENGTH];

void parse_client_command(char *input) ;

int main(int argc, char** argv) {
    struct sockaddr_in sa;
    int fd_skt, portno, result;
    char* address, input[512];
    ssize_t nread;
    fd_set set, buf_set;

    struct timeval timeout, timeout_buf;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (argc < 3) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    address = argv[1];
    portno = atoi(argv[2]);

    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t) portno);
    sa.sin_addr.s_addr = inet_addr(address);
    fd_skt = socket(AF_INET, SOCK_STREAM, 0);

    get_uuid(uuid);

    if (connect(fd_skt, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
       dprintf(STDERR_FILENO, "Can't connect to %s:%d, Exiting...", address, portno);
        exit(-1);
    }

    FD_SET(fd_skt, &set);

    bool running = true;
    while (running) {
        memset(input, 0, sizeof(input));

        scanf("%s", input);
        if (!strcmp(input, COMMAND_CLOSE)) {
            break;
        }
        parse_client_command(input);

        send_data(fd_skt, input, (int) strlen(input), 0);
        memset(input, 0, sizeof(input));

        timeout_buf = timeout;
        buf_set = set;
        select(fd_skt + 1, &buf_set, 0, 0, &timeout_buf);
        if (FD_ISSET(fd_skt, &buf_set)) {
            read(fd_skt, input, sizeof(input));
            printf("%s\n", input);
        } else {
            printf("Connection is lost...");
        }
    }

    close(fd_skt);
    exit(EXIT_SUCCESS);
}

void parse_client_command(char *input) {

    if (input == strstr(input, COMMAND_DOWNLOAD)) {
        strcat(input, " ");
        strcat(input, uuid);
    }
    strcat(input, COMMAND_END_1);
}