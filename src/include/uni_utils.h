#ifndef HTMPFS_UNI_UTILS_H
#define HTMPFS_UNI_UTILS_H

#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>

/// get current time
timespec get_current_time()
{
    timespec ts{};
    timespec_get(&ts, TIME_UTC);
    return ts;
}

#endif //HTMPFS_UNI_UTILS_H
