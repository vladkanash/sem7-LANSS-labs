//
// Created by vladkanash on 1.11.16.
//

#ifndef LANSS_TYPES_H
#define LANSS_TYPES_H

#include <stdbool.h>
#include <wchar.h>
#include "constants.h"

typedef enum commands {TIME, ECHO, CLOSE, DOWNLOAD, KILL} commands;

typedef enum server_state {INITIAL, UPLOADING, PARSING} server_state;

typedef struct client_session {
    char* text;
    char uuid[UUID_LENGTH];
    bool simple;
    bool success;
    commands type;
    size_t command_length;
    server_state state;
} client_session;

typedef struct command_response {
    char* text;
    unsigned text_length;
    commands type;
    bool success;
    server_state next_state;
} command_response;

typedef struct command_holder {
	char* name;
	unsigned length;
	bool simple;
	commands type;
} command_holder;

#endif //LANSS_TYPES_H
