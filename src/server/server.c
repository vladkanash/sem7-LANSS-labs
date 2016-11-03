//
// Created by vladkanash on 7.10.16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "server.h"
#include "engine.h"
#include "../constants.h"

int fd_hwm = 0;
fd_set set;
server_command command_list[256];

void run_server(struct sockaddr_in *sap) {
    int fd_skt, fd;
    fd_set read_set;

    fd_skt = socket(AF_INET, SOCK_STREAM, 0);

    if (bind(fd_skt, (struct sockaddr*) sap, sizeof(*sap)) < 0) {
        perror("error while binding socket");
        exit(-1);
    }

    if (listen(fd_skt, SOMAXCONN) < 0) {
        perror("error while trying to listen");
        exit(-1);
    }

    if (fd_skt > fd_hwm) {
        fd_hwm = fd_skt;
    }

    FD_ZERO(&set);
    FD_SET(fd_skt, &set);

    bool running = true;
    while (running) {
        read_set = set;
        select(fd_hwm + 1, &read_set, NULL, NULL, NULL);
        for (fd = 0; fd <= fd_hwm; fd++) {
            if (FD_ISSET(fd, &read_set)) {
                if (fd == fd_skt) {
                    add_client(fd);
                } else {
                    input_data(fd);
                }
            }
        }
    }
    close(fd_skt);
    return;
}

void add_client(int fd) {
    int fd_client = accept(fd, NULL, 0);
    FD_SET(fd_client, &set);
    if (fd_client > fd_hwm) {
        fd_hwm = fd_client;
    }
}

void input_data(int fd) {
    char in_buf[BUF_SIZE];
    char out_buf[BUF_SIZE];
    memset(in_buf, 0, sizeof(in_buf));
    memset(out_buf, 0, sizeof(out_buf));
    ssize_t nread = 0;
    server_command command;
    command_response response;

    server_state current_state = command_list[fd].state;
    switch (current_state) {
        case INITIAL : {
            do {
                nread = recv(fd, in_buf, sizeof(in_buf), MSG_PEEK);
                command = get_command(in_buf);
                if (command.success) {
                    command_list[fd] = command;
                    if (command.simple) {
                        response = process_command(command);
                        if (response.type == CLOSE) {
                            cleanup(in_buf, out_buf, &command, &response);
                            close_connection(fd);
                        }
                        recv(fd, in_buf, command.text + command.command_length - in_buf, NULL); //remove simple command
                    } else {
                        command_list[fd].state = PARSING;
                        recv(fd, in_buf, command.text + command.command_length - in_buf, NULL); //remove first part of long command
                        break;
                    }
                    write(fd, response.text, response.text_length);
                    cleanup(in_buf, out_buf, &command, &response);
                } else if (nread > COMMAND_MAX_LENGTH) {
                    recv(fd, in_buf, (size_t) (nread - COMMAND_MAX_LENGTH), NULL); //remove garbage text with no commands found
                }
            } while (nread == sizeof(in_buf));
            break;
        }
        case PARSING : {
            size_t old_size, read_size = BUF_SIZE;
            char* buf = malloc(sizeof(char) * read_size);
            command = command_list[fd];
            do {
                old_size = read_size;
                nread = recv(fd, buf, read_size, MSG_PEEK);

                if (get_long_command(buf, &command)) {
                    response = process_command(command);
                    command_list[fd].state = response.next_state;
                    recv(fd, buf, command.command_length, NULL); //remove text part of long command
                    write(fd, response.text, response.text_length);
                    cleanup(in_buf, out_buf, &command, &response);
                } else {
                    read_size *= 2;
                    buf = realloc(buf, sizeof(char) * read_size);
                }
            } while (nread == old_size);
            free(buf);
            break;
        }
        case UPLOADING : {
            break;
        }
    }
}

void close_connection(int fd) {
    FD_CLR(fd, &set);
    if (fd == fd_hwm) {
        fd_hwm--;
    }
    close(fd);
}

void download_file(char *buf, char *out_buf, int fd) {
    int offset;
    ssize_t sent_bytes;
    __off_t remain_data;
    struct stat file_stat;
    memmove(buf, buf + sizeof(COMMAND_DOWNLOAD) - 1, BUF_SIZE);
    buf[strlen(buf) - 1] = '\0';

    int file = open(buf, O_RDONLY);
    if (file == -1) {
        memset(out_buf, 0, BUF_SIZE);
        sprintf(out_buf, "File %s cannot be found on server", buf);
        perror("Can't open file");
        return;
    }
    if (fstat(file, &file_stat) < 0) {
        perror("fstat error");
        return;
    }
    fprintf(stdout, "File size = %li bytes", file_stat.st_size);

    offset = 0;
    remain_data = file_stat.st_size;
    while ((sent_bytes = sendfile(fd, file, (off_t *) &offset, BUF_SIZE)) > 0) {
        fprintf(stdout, "Server sent %zi bytes from file's data, offset is now : %d and remaining data is  %li\n",
        sent_bytes, offset, remain_data);
        remain_data -= sent_bytes;
    }
}
