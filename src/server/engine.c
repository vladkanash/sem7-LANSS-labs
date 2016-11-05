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
    memset(command, 0, sizeof(server_command));
    memset(response, 0, sizeof(command_response));
}

bool parse_long_command(char *input, char *command_type, int command_len, server_command* command) {
    if ((command->text = strstr(input, command_type))) {
        command->command_length = (size_t) command_len;
    }
    return NULL != command->text;
}

server_command get_command(char *buf) {
    server_command result[COMMAND_COUNT];
    server_command* response = &result[0];
    for (int i = 0; i < COMMAND_COUNT; i++) {
        memset(&result, 0, sizeof(result));
        result[i].success = false;
    }
    if (parse_long_command(buf, COMMAND_ECHO, sizeof(COMMAND_ECHO) - 1, &result[0])) {
        result[0].type = ECHO;
        result[0].simple = false;
        result[0].success = true;
    }else if (parse_command(buf, COMMAND_TIME, sizeof(COMMAND_TIME) - 1, &result[1])) {
        result[1].type = TIME;
        result[1].simple = true;
        result[1].success = true;
    } else if (parse_command(buf, COMMAND_CLOSE, sizeof(COMMAND_CLOSE) - 1, &result[2])) {
        result[2].type = CLOSE;
        result[2].simple = true;
        result[2].success = true;
    } else if (parse_long_command(buf, COMMAND_DOWNLOAD, sizeof(COMMAND_DOWNLOAD) - 1, &result[3])) {
        result[3].type = DOWNLOAD;
        result[3].simple = false;
        result[3].success = true;
    }
    for (int i = 1; i < COMMAND_COUNT; i++) {
        if (result[i].text != NULL) {
            response = &result[i];
        }
    }
    for (int i = 1; i < COMMAND_COUNT; i++) {
        if (result[i].text != NULL && result[i].text < response->text) {
            response = &result[i];
        }
    }
    response->state = INITIAL;
    return *response;
}

bool get_long_command(char* buf, server_command *command) {
    int end_length = 2;
    char* start = strstr(buf, COMMAND_END_2);
    if (NULL == start) {
        start = strstr(buf, COMMAND_END_1);
        end_length = 1;
    }
    if (NULL != start) {
        unsigned long size = start - buf + end_length;
        command->command_length = size;
        command->text = malloc(size);
        strncpy(command->text, buf, size);
        return true;
    }
    return false;
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
    result.next_state = INITIAL;

    switch (command.type) {
        case TIME : {
            result.type = command.type;
            get_current_time(&result);
            result.text_length = 30;
            result.success = true;
            break;
        }
        case CLOSE : {
            result.type = command.type;
            result.text = malloc(23 * sizeof(char));
            sprintf(result.text, "Closing connection...\r\n");
            result.success = true;
            break;
        }
        case ECHO : {
            result.type = command.type;
            result.text_length = (unsigned int) command.command_length;
            result.text = malloc(command.command_length * sizeof(char));
            strcpy(result.text, command.text);
            free(command.text);
            result.success = true;
            break;
        }
        case DOWNLOAD : {
            result.type = command.type;
            result.text_length = (unsigned int) command.command_length + 28;
            result.text = malloc((command.command_length + 28) * sizeof(char));
            sprintf(result.text, "Request to download file: %s\r\n", command.text);
            result.next_state = UPLOADING;
            result.success = true;
            break;
        }
        default : {
            result.success = false;
            break;
        }
    }
    return result;
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
    memset(result->text, 0, size);
    strftime(result->text , size, "%Y-%m-%d %X\n", tm);
    result->text_length = size;
}
