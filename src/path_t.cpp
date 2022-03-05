#include <path_t.h>

path_t::path_t(const std::string & given_path)
{
    std::string cur;
    std::string path_to_file = given_path;
    path_to_file.erase(path_to_file.begin());

    for (auto & i : path_to_file)
    {
        if (i == '/')
        {
            pathname.append(cur);
            cur.clear();
            continue;
        }

        cur += i;
    }

    if (!cur.empty())
    {
        pathname.append(cur);
    }
}
