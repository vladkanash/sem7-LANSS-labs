//
// Created by vladkanash on 1.11.16.
//

#ifndef LANSS_TYPES_H
#define LANSS_TYPES_H

#include <stdbool.h>
#include <wchar.h>
#include "constants.h"

typedef enum commands {TIME, ECHO, CLOSE, DOWNLOAD, KILL} commands;

typedef enum server_state {INITIAL, IDLE, START_UPLOADING, UPLOADING, ECHOING} server_state;

typedef enum client_state {INITIAL, IDLE, DOWNLOADING} client_state;

typedef struct server_command {
    char* text;
    char uuid[UUID_LENGTH + 1];
    bool simple;
    bool success;
    commands type;
    size_t command_length;
    server_state state;
} server_command;

typedef struct download_handler {
    char uuid[UUID_LENGTH];
    char* path;
    size_t offset;
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
    char* text;
    unsigned text_length;
    bool success;
    server_state next_state;
} command_response;

typedef struct command_holder {
	char* name;
	unsigned length;
	bool simple;
	commands type;
} command_holder;

typedef struct file_info {
    char name[BUF_SIZE];
    size_t size;
    size_t offset;
} file_info;
#endif //LANSS_TYPES_H
