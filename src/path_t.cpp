#include <htmpfs/path_t.h>

path_t::path_t(const std::string & given_path)
{
    pathname.emplace_back("");

    std::string cur;
    std::string path_to_file = given_path;

    for (auto & i : path_to_file)
    {
        if (i == '/')
        {
            if (cur.empty())
            {
                continue;
            }

            pathname.emplace_back(cur);
            cur.clear();
            continue;
        }

        cur += i;
    }

    if (!cur.empty())
    {
        pathname.emplace_back(cur);
    }
}

std::string path_t::to_string()
{
    std::string ret;
    for (const auto& i : pathname)
    {
        // ignore root
        if (i.empty()) { continue; }

        ret += "/" + i;
    }
    return ret;
}
