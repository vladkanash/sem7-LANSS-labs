//
// Created by vladkanash on 7.10.16.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdbool.h>
#include <libgen.h>

#include "../constants.h"
#include "../system_dependent_code.h"
#include "server.h"
#include "engine.h"
#include "download_list.h"
#include "session_container.h"

int fd_hwm = 0;
fd_set read_set, write_set;

void run_server(struct sockaddr_in *sap) {
    int fd_skt, fd;
    fd_set read_buf, write_buf;
    struct timeval timeout_buf;

    init_commands();
    init_stop_handler();
	fd_skt = initialize_socket();

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
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100;

    FD_ZERO(&read_set);
    FD_SET(fd_skt, &read_set);

    bool running = true;
    while (running) {
        read_buf = read_set;
        write_buf = write_set;
        timeout_buf = timeout;
        select(fd_hwm + 1, &read_buf, &write_buf, 0, &timeout_buf);
        for (fd = 0; fd <= fd_hwm; fd++) {
            if (FD_ISSET(fd, &read_buf) || FD_ISSET(fd, &write_buf)) {
                fd == fd_skt ? add_client(fd) : input_data(fd);
            }
        }
    }
    close_socket(fd_skt);
    quit_socket();
    return;
}

void input_data(int fd) {
    session_handler* session = get_session_by_fd(fd);
    server_state current_state = session != NULL ? session->state : INITIAL;
    switch (current_state) {
        case INITIAL : {
            init_client_session(fd);
            break;
        }
        case IDLE : {
            parse_command_start(session);
            break;
        }
        case ECHOING : {
            process_echo(session);
            break;
        }
        case START_UPLOADING : {
            start_file_upload(session);
            break;
        }
        case UPLOADING : {
            upload_file_part(session);
        }
    }
}

void add_client(int fd) {
    int fd_client = accept(fd, 0, 0);
    FD_SET(fd_client, &read_set);
    if (fd_client > fd_hwm) {
        fd_hwm = fd_client;
    }
}

void start_file_upload(session_handler* session) {
    struct stat file_stat;
    static char out_buf[BUF_SIZE];
    static char file_path[BUF_SIZE];
    static char* file_name;
    static file_info info;
    int fd = session->fd;

    memset(file_path, 0, BUF_SIZE);
    memset(&info, 0, sizeof(file_info));

    get_file_path(session, file_path);

    file_name = basename(file_path);
    strcpy(info.name, file_name);

    int file = open(file_path, O_RDONLY);
    if (file == -1) {
        perror("Can't open file");
        memset(out_buf, 0, BUF_SIZE);
        sprintf(out_buf, "Cannot find file on server: %s ", file_path);
        write(fd, out_buf, BUF_SIZE);
        session->state = IDLE;
        return;
    }
    if (fstat(file, &file_stat) < 0) {
        perror("fstat error");
		memset(out_buf, 0, BUF_SIZE);
        sprintf(out_buf, "There was an error opening file %s", file_path);
        write(fd, out_buf, BUF_SIZE);
        session->state = IDLE;
        return;
    }
    fprintf(stdout, "File size = %li bytes", file_stat.st_size);
    info.size = (size_t) file_stat.st_size;
    FD_SET(fd, &write_set);

    add_download(session->uuid, file);
    download_handler* download = get_download(session->uuid);
    download -> size = file_stat.st_size;
    session->download = download;

    send_data(fd, &info, sizeof(file_info), NULL);
    upload_file_part(session); //uploading first part of file.
}

void get_file_path(const session_handler *session, char *file_path) {
    int fd = session->fd;
    recv(fd, file_path, BUF_SIZE, MSG_PEEK);
    file_path[strlen(file_path) - 1] = '\0';

    if (file_path[strlen(file_path) - 1] == '\r') {
        file_path[strlen(file_path) - 1] = '\0';
    }
}

void parse_command_start(session_handler* session) {
    static char in_buf[BUF_SIZE];
    static command_response response;
    static int fd;

    memset(&response, 0, sizeof(command_response));
    memset(in_buf, 0, sizeof(in_buf));

    server_command* com = &(session->command);
    fd = session->fd;

    recv(fd, in_buf, BUF_SIZE, MSG_PEEK);                   // 0: read data from socket
    *com = get_command(in_buf, 0);                          // 1: parse command from data
    response = process_command(com);                        // 2: process command and generate response
    session->state = response.next_state;                   // 3: update session state according to response
    send_data(fd, response.text, response.text_length, 0);  // 4: send response data back to client
    flush_socket(session);                                  // 5: remove processed data from socket

    if (com->success == true && com->type == CLOSE) {
        close_connection(fd);
        return;
    }
}

void process_echo(session_handler* session) {
    size_t command_end = 0;
    int fd = session->fd;
    static char buf[BUF_SIZE];

    memset(buf, 0, sizeof(buf));

    recv(fd, buf, BUF_SIZE, MSG_PEEK);
    command_end = find_line_ending(buf, BUF_SIZE);

    recv(fd, buf, command_end, 0);
    send_data(fd, buf, (int) command_end, 0);
    if (command_end < BUF_SIZE) {
        session->state = IDLE;
        return;
    }
}

void upload_file_part(session_handler* session) {
    download_handler* download = session->download;
    static int fd;
    fd = session->fd;

    if (NULL == download) {
        perror("ERROR: Can't find download with uuid specified.");
        session->state = IDLE;
        return;
    }

    off_t offset = download->offset;
    size_t remain_data = download->size - download->offset;
    size_t bytes_to_send = remain_data > CHUNK_SIZE ? CHUNK_SIZE : remain_data;

    ssize_t sent_bytes = sendfile(fd, download->file, &offset, (size_t) bytes_to_send);

//    fprintf(stdout, "Server sent %zi bytes from file's data, offset is now : %d and remaining data is  %li\n",
//            bytes_to_send, (int)offset, remain_data - sent_bytes);

    if (sent_bytes == remain_data) {
        printf("Server sent %zi bytes, File successfully uploaded.", download->size);
        FD_CLR(fd, &write_set);
        session->state = IDLE;
        remove_download(session->uuid);
        close(download->file);
    } else {
        download -> offset += (size_t) sent_bytes;
        session->state = UPLOADING;
    }
}

void init_client_session(int fd) {
    static char uuid[UUID_LENGTH];
    memset(uuid, 0, UUID_LENGTH);
    recv(fd, uuid, UUID_LENGTH, NULL);

    session_handler* session = get_session(uuid);
    if (session != NULL) {
        session->fd = fd;
        return;
    }

    session_handler new_session;
    new_session.fd = fd;
    new_session.state = IDLE;
    memcpy(new_session.uuid, uuid, UUID_LENGTH);
    put_session(&new_session);
}

void flush_socket(session_handler* session) {
    static char buf[BUF_SIZE];
    size_t size = session->command.command_length;

    ssize_t nread = 0;
    do {
        size -= nread;
        nread = recv(session->fd, buf, size, 0);
    } while (size != nread);
}

void close_connection(int fd) {
    FD_CLR(fd, &read_set);
    if (fd == fd_hwm) {
        fd_hwm--;
    }
    close_socket(fd);
}

void stop_server() {
    for (int fd = 0; fd <= fd_hwm; fd++) {
        if (FD_ISSET(fd, &read_set) || FD_ISSET(fd, &write_set)) {
            close_socket(fd);
        }
        quit_socket();
    }
    exit(0);
}