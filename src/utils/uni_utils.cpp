/** @file
 *
 * This file implements the frequently used utilities
 */

#include <uni_utils.h>

timespec get_current_time()
{
    timespec ts{};
    timespec_get(&ts, TIME_UTC);
    return ts;
}

std::string make_path_with_version(const std::string& path, const snapshot_ver_t& version)
{
    if (version == FILESYSTEM_CUR_MODIFIABLE_VER)
    {
        return path;
    }

    std::string ret;
    ret += "/.snapshot/" + version;
    ret += path;
    return ret;
}
