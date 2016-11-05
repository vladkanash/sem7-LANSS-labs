//
// Created by vladkanash on 1.11.16.
//

#ifndef LANSS_TYPES_H
#define LANSS_TYPES_H

#include <stdbool.h>

typedef enum commands {TIME, ECHO, CLOSE, DOWNLOAD, KILL} commands;

typedef enum server_state {INITIAL, UPLOADING, PARSING} server_state;

typedef struct server_command {
    char* text;
    bool simple;
	bool success;
	commands type;
	size_t command_length;
	server_state state;
} server_command;

typedef struct command_response {
    char* text;
    unsigned text_length;
    commands type;
    bool success;
    server_state next_state;
} command_response;

#endif //LANSS_TYPES_H
