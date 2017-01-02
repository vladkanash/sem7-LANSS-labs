//
// Created by vladkanash on 1.11.16.
//

#ifndef LANSS_TYPES_H
#define LANSS_TYPES_H

#include <stdbool.h>
#include <wchar.h>
#include <sys/param.h>
#include "constants.h"

typedef enum command_type {TIME, ECHO, CLOSE, DOWNLOAD, KILL} command_type;

typedef enum server_state {INITIAL, IDLE, START_UPLOADING, UPLOADING, ECHOING} server_state;

typedef enum client_state {CLIENT_IDLE, CLIENT_DOWNLOADING, CLIENT_ECHOING} client_state;

typedef struct server_command {
    char text[256];
    bool simple;
    bool success;
    command_type type;
    size_t command_length;
	server_state next_state;
} server_command;

typedef struct download_handler {
	char uuid[UUID_LENGTH];
    char path[256];
    unsigned long long offset;
    int file;
    long size;
} download_handler;

typedef struct session_handler {
	server_command command;
    download_handler* download;
	char uuid[UUID_LENGTH];
	server_state state;
    int fd;
} session_handler;

typedef struct command_response {
    char text[256];
    bool success;
} command_response;

typedef struct command_holder {
	char* name;
	unsigned length;
	bool simple;
	command_type type;
	server_state next_state;
} command_holder;

typedef struct file_info {
    char name[BUF_SIZE];
	char comment[256];
    size_t size;
	off_t offset;
} file_info;

typedef struct file_request {
	char path[BUF_SIZE];
	off_t offset;
} file_request;
#endif //LANSS_TYPES_H
