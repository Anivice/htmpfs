#include <uni_utils.h>

timespec get_current_time()
{
    timespec ts{};
    timespec_get(&ts, TIME_UTC);
    return ts;
}

std::string make_path_with_version(const std::string& path, snapshot_ver_t version)
{
    if (version == 0)
    {
        return path;
    }

    std::string ret;
    ret += "/.snapshot/" + std::to_string(version);
    ret += path;
    return ret;
}
