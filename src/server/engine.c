//
// Created by vladkanash on 2.11.16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "engine.h"
#include "../constants.h"

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
        result.type = command.type;
        get_current_time(&result);
        result.text_length = 30;
        result.success = true;
    } else if (command.type == CLOSE) {
        result.type = command.type;
        result.success = true;
    } else {
        result.success = false;
    }
    return result;
}

void echo_message(char *in_buf, char *out_buf) {
    memcpy(out_buf, in_buf, BUF_SIZE);
    memmove(out_buf, out_buf+5, BUF_SIZE);
}

bool startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
            lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

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