#pragma once

#include <pebble.h>

struct stat {
    int32_t cur;
    int32_t max;
    char label[10];
};

status_t save_stat(const uint32_t key, struct stat *data);
status_t read_stat(const uint32_t key, struct stat *data);
