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

#include "../constants.h"
#include "../system_dependent_code.h"
#include "server.h"
#include "engine.h"
#include "download_list.h"

int fd_hwm = 0;
fd_set read_set, write_set;

client_session sessions[MAX_CLIENTS];

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
    server_state current_state = sessions[fd].state;
    switch (current_state) {
        case INITIAL : {
            parse_command_start(fd);
            break;
        }
        case PARSING : {
            parse_command_end(fd);
            break;
        }
        case START_UPLOADING : {
            start_file_upload(fd);
            break;
        }
        case UPLOADING : {
            upload_file_part(fd);
        }
    }
}

void add_client(int fd) {
    int fd_client = accept(fd, 0, 0);
    FD_SET(fd_client, &read_set);
    fd_client > fd_hwm && (fd_hwm = fd_client);
}

void start_file_upload(int fd) {
    client_session session = sessions[fd];
    char* file_path = session.text;

	struct stat file_stat;
    static char out_buf[BUF_SIZE];

    int file = open(file_path, O_RDONLY);
    if (file == -1) {
        perror("Can't open file");
        memset(out_buf, 0, BUF_SIZE);
        sprintf(out_buf, "Cannot find file on server: %s ", file_path);
        write(fd, out_buf, BUF_SIZE);
        sessions[fd].state = INITIAL;
        return;
    }
    if (fstat(file, &file_stat) < 0) {
        perror("fstat error");
		memset(out_buf, 0, BUF_SIZE);
        sprintf(out_buf, "There was an error opening file %s", file_path);
        write(fd, out_buf, BUF_SIZE);
        sessions[fd].state = INITIAL;
        return;
    }
    fprintf(stdout, "File size = %li bytes", file_stat.st_size);
    FD_SET(fd, &write_set);

    add_download(session.uuid, file);
    download_handler* download = get_download(session.uuid);
    download -> size = file_stat.st_size;

    upload_file_part(fd); //uploading first part of file.
}

void parse_command_start(int fd) {
    static char in_buf[BUF_SIZE];
    static char out_buf[BUF_SIZE];
    memset(in_buf, 0, sizeof(in_buf));
    memset(out_buf, 0, sizeof(out_buf));
    client_session command;
    command_response response;
    ssize_t nread = 0;
    do {
        nread = recv(fd, in_buf, BUF_SIZE, MSG_PEEK);
        command = get_command(in_buf);
        if (command.success) {
            sessions[fd] = command;
            if (command.simple) {
                response = process_command(command);
                if (response.type == CLOSE) {
                    send_data(fd, response.text, response.text_length, 0);
                    close_connection(fd);
                }
                recv(fd, in_buf, command.command_length, 0); //remove simple command
            } else {
                sessions[fd].state = PARSING;
                recv(fd, in_buf, command.command_length, 0); //remove first part of long command
                break;
            }
			send_data(fd, response.text, response.text_length, 0);
			free(response.text);
        } else {
            size_t ndel = find_line_ending(in_buf);
            recv(fd, in_buf, ndel, 0); //remove garbage text with no commands found
        }
    } while (nread == sizeof(in_buf));
}

void parse_command_end(int fd) {
    client_session session;
    command_response response;
    ssize_t nread;
    size_t old_size, read_size = BUF_SIZE;
    char* buf = (char*)malloc(sizeof(char) * read_size);
    session = sessions[fd];

    do {
        old_size = read_size;
        nread = recv(fd, buf, read_size, MSG_PEEK);

        if (get_long_command(buf, &session)) {
            check_download_arguments(&session);
            response = process_command(session);
            recv(fd, buf, session.command_length, 0); //remove text part of long session
            session.state = response.next_state;
            sessions[fd] = session;
			send_data(fd, response.text, response.text_length, 0);
			free(response.text);
        } else {
            read_size *= 2;
            buf = (char*)realloc(buf, sizeof(char) * read_size);
        }
    } while (nread == old_size);
    free(buf);
}

void upload_file_part(int fd) {
    client_session session = sessions[fd];

    download_handler* download = get_download(session.uuid);
    if (NULL == download) {
        perror("ERROR: Can't find download with uuid specified.");
        sessions[fd].state = INITIAL;
        return;
    }

    int file = download->file;
    off_t offset = download->offset;
    size_t remain_data = download->size - download->offset;
    size_t bytes_to_send = remain_data > CHUNK_SIZE ? CHUNK_SIZE : remain_data;

    ssize_t sent_bytes = sendfile(fd, file, &offset, (size_t) bytes_to_send);

    fprintf(stdout, "Server sent %zi bytes from file's data, offset is now : %d and remaining data is  %li\n",
            bytes_to_send, (int)offset, remain_data - sent_bytes);

    if (sent_bytes == remain_data) {
        FD_CLR(fd, &write_set);
        sessions[fd].state = INITIAL;
        remove_download(session.uuid);
        close(file);
    } else {
        download -> offset += (size_t) sent_bytes;
        sessions[fd].state = UPLOADING;
    }
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