//
// Created by vladkanash on 2.1.17.
//

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "util.h"

void get_size_str(double size, char* str) {
    if (size > 1024 * 1024 * 1024) {
        sprintf(str, "%.3f GB", size / (1024 * 1024 * 1024));
    } else if (size > 1024 * 1024) {
        sprintf(str, "%.3f MB", size / (1024 * 1024));
    } else if (size > 1024) {
        sprintf(str, "%.3f KB", size / 1024);
    } else sprintf(str, "%f B", size);
}

bool starts_with(const char *str, const char *pre) {
    size_t lenpre = strlen(pre),
            lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}