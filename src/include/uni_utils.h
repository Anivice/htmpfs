#ifndef HTMPFS_UNI_UTILS_H
#define HTMPFS_UNI_UTILS_H

#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include <htmpfs_types.h>
#include <string>

/// get current time
timespec get_current_time();
std::string make_path_with_version(const std::string& path, snapshot_ver_t version);
// check is given path starts with /.snapshot/$(version)/
snapshot_ver_t if_snapshot(const std::string & path, std::string & output);

#endif //HTMPFS_UNI_UTILS_H
