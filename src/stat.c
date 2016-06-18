#include <pebble.h>
#include "stat.h"

status_t save_stat(const uint32_t key, struct stat *data)
{
    if (key % 2 != 0)
        return E_INVALID_ARGUMENT;
    persist_write_int(key,   data->cur);
    persist_write_int(key+1, data->max);
    return S_SUCCESS;
}

status_t read_stat(const uint32_t key, struct stat *data)
{
    if (!persist_exists(key) || !persist_exists(key+1))
        return E_DOES_NOT_EXIST;
    data->cur = persist_read_int(key);
    data->max = persist_read_int(key+1);
    return S_SUCCESS;
}
