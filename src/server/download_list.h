//
// Created by vladkanash on 13.11.16.
//

#include "stdbool.h"
#include "types.h"
#include <string.h>

#ifndef LANSS_UUID_LIST_H
#define LANSS_UUID_LIST_H

void add_download(char *uuid, int file);
void remove_download(char *uuid);
bool download_exists(char *uuid);
download_handler* get_download(char *uuid);

#endif //LANSS_UUID_LIST_H
