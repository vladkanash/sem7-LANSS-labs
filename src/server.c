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
#include <time.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "commands.h"
#include "types.h"

#define BUF_SIZE 256
#define COMMAND_LENGTH 16

int fd_hwm = 0;
fd_set set;

void echo_message(char *in_buf, char *out_buf);
command_response process_command(server_command command);
void download_file(char *buf, char *out_buf, int i);
server_command get_command(char *buf);
void close_connection(int fd);
void cleanup(char *in_buf, char *out_buf, server_command *command, command_response *response);
void input_data(int fd);
void add_client(int fd_skt);

bool parse_command(char *input, char command_type[5], int command_len, server_command* command);

void get_current_time(command_response* result) {
    unsigned size = 30;
    struct tm *tm;
    time_t t;
    t = time(NULL);
    tm = localtime(&t);
    result->text = malloc(sizeof(char) * size);
    strftime(result->text , size, "%F %X\n", tm);
    result->text_length = size;
}

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
            lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

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
                    add_client(fd_skt);
                } else {
                    input_data(fd);
                }
            }
        }
    }
    close(fd_skt);
    return;
}

void add_client(int fd_skt) {
    int fd_client = accept(fd_skt, NULL, 0);
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

    do {
        nread = recv(fd, in_buf, sizeof(in_buf), MSG_PEEK);
        command = get_command(in_buf);
        if (command.success) {
            response = process_command(command);
            if (response.close_connection) {
                close_connection(fd);
            }
            recv(fd, in_buf, command.text + command.command_length - in_buf, 0);
            write(fd, response.text, response.text_length);
            cleanup(in_buf, out_buf, &command, &response);
        } else if (nread > COMMAND_LENGTH) {
            recv(fd, in_buf, (size_t) (nread - COMMAND_LENGTH), 0);
        }
    } while (nread == sizeof(in_buf));
}

void cleanup(char *in_buf, char *out_buf, server_command *command, command_response *response) {
    memset(out_buf, 0, BUF_SIZE);
    memset(in_buf, 0, BUF_SIZE);
    free(response->text);
    memset(command, 0, sizeof(server_command));
    memset(response, 0, sizeof(command_response));
}

server_command get_command(char *buf) {
    server_command result;
    memset(&result, 0, sizeof(result));
    result.success = true;
    if (parse_command(buf, COMMAND_TIME, COMMAND_TIME_LENGTH, &result)) {
        result.type = TIME;
        result.simple = true;
    } else if (parse_command(buf, COMMAND_ECHO, COMMAND_ECHO_LENGTH, &result)) {
        result.type = ECHO;
        result.simple = false;
    } else if (parse_command(buf, COMMAND_CLOSE, COMMAND_CLOSE_LENGTH, &result)) {
        result.type = CLOSE;
        result.simple = true;
    } else if (parse_command(buf, COMMAND_DOWNLOAD, COMMAND_DOWNLOAD_LENGTH, &result)) {
        result.type = DOWNLOAD;
        result.simple = false;
    } else {
        result.success = false;
    }
    return result;
}

bool parse_command(char *input, char *command_type, int command_len, server_command* command) {
    bool long_ending = false;
    bool little_ending = false;
    if ((command->text = strstr(input, command_type))) {
        long_ending = command->text[command_len] == '\r' && command->text[command_len + 1] == '\n';
        little_ending = command->text[command_len] == '\n';
        command->command_length = (unsigned int) (long_ending ? command_len + 2 : little_ending ? command_len + 1 : 0);
    }
    return (command->text && (long_ending || little_ending));
}

command_response process_command(server_command command) {
    command_response result;
    memset(&result, 0, sizeof(result));
    if (command.type == TIME) {
        get_current_time(&result);
        result.text_length = 30;
        result.success = true;
        result.close_connection = false;
    } else if (command.type == CLOSE) {
        result.close_connection = true;
        result.success = true;
    } else {
        result.success = false;
        result.close_connection = false;
    }
    return result;
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

void echo_message(char *in_buf, char *out_buf) {
    memcpy(out_buf, in_buf, BUF_SIZE);
    memmove(out_buf, out_buf+5, BUF_SIZE);
}


