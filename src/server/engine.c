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

static command_holder command_info[COMMAND_COUNT];

void init_commands() {
    command_info[0].type = ECHO;
    command_info[0].simple = false;
    command_info[0].length = sizeof(COMMAND_ECHO) - 1;
    command_info[0].name = COMMAND_ECHO;

    command_info[1].type = DOWNLOAD;
    command_info[1].simple = false;
    command_info[1].length = sizeof(COMMAND_DOWNLOAD) - 1;
    command_info[1].name = COMMAND_DOWNLOAD;

    command_info[2].type = TIME;
    command_info[2].simple = true;
    command_info[2].length = sizeof(COMMAND_TIME) - 1;
    command_info[2].name = COMMAND_TIME;

    command_info[3].type = CLOSE;
    command_info[3].simple = true;
    command_info[3].length = sizeof(COMMAND_CLOSE) - 1;
    command_info[3].name = COMMAND_CLOSE;
}

server_command get_command(char *buf) {
    server_command response;
    memset(&response, 0, sizeof(server_command));
    response.state = INITIAL;
    response.success = false;

    parse_command(buf, &response);
    return response;
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
        command->text = (char*)malloc(size);
        strncpy(command->text, buf, size);
        return true;
    }
    return false;
}

bool string_contain_end_line_symb(char* string){
    return string[strlen(string)-1] == '\n';
}

bool string_contain_end_line_and_return_symb(char* string){
    return string_contain_end_line_symb(string) && string[strlen(string)-2] == '\r';
}

bool is_command_name(char* input, char* command_name){
   return strstr(input, command_name) == input && 
        ( (strlen(input) <= strlen(command_name) + 1 && string_contain_end_line_symb(input)) ||
         (strlen(input) <= strlen(command_name) + 2 && string_contain_end_line_and_return_symb(input)) );
}

bool parse_command(char *input, server_command* command) {
    for (int i = 0; i < COMMAND_COUNT; i++) {
        command_holder com = command_info[i];
        command->type = com.type;
        command->simple = com.simple;
        
        if (is_command_name(input, com.name)){
            command->success = true;
            command->text = com.name;
            command->command_length = (size_t) (command->simple ? com.length + 1 : com.length);
            return true;
        }

    }
    return false;
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
            result.text = (char*)malloc(24 * sizeof(char));
            result.text_length = 24;
            sprintf(result.text, "Closing connection...\r\n");
            result.success = true;
            break;
        }
        case ECHO : {
            result.type = command.type;
            result.text_length = (unsigned int) command.command_length;
            result.text = (char*)malloc(command.command_length * sizeof(char));
            strcpy(result.text, command.text);
            free(command.text);
            result.success = true;
            break;
        }
        case DOWNLOAD : {
            result.type = command.type;
            result.text_length = (unsigned int) command.command_length + 28;
            result.text = (char*)malloc((command.command_length + 28) * sizeof(char));
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

size_t find_line_ending(char *buf) {
    int ending_len = 0;
    char* start = strstr(buf, COMMAND_END_2); {
        ending_len = 2;
        if (NULL == start) {
            ending_len = 1;
            start = strstr(buf, COMMAND_END_1);
        }
    }
    if (NULL != start) {
        return start - buf + ending_len;
    } else {
        return 0;
    }
}

void get_current_time(command_response* result) {
    unsigned size = 30;
    struct tm *tm;
    time_t t;
    t = time(NULL);
    tm = localtime(&t);
    result->text = (char*)malloc(sizeof(char) * size);
    memset(result->text, 0, size);
    strftime(result->text , size, "%Y-%m-%d %X\n", tm);
    result->text_length = size;
}
