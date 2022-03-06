#include <directory_resolver.h>
#include <path_t.h>
#include <algorithm>
#include <htmpfs_error.h>

directory_resolver_t::directory_resolver_t(const std::string &_path, inode_t *_associated_inode)
: associated_inode(_associated_inode)
{
    path_t split_path(_path);

    for (const auto& i : split_path)
    {
        path.emplace_back(i);
    }

    path.erase(path.begin());
}

std::vector<std::string> directory_resolver_t::to_vector()
{
    return path;
}

void directory_resolver_t::add_path(const std::string & pathname)
{
    if (std::find(path.begin(), path.end(), pathname) != path.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_DOUBLE_MKPATHNAME);
    }

    path.emplace_back(pathname);
}

std::string directory_resolver_t::to_string()
{
    std::string ret("/");

    for (const auto& i : path)
    {
        ret += i + '/';
    }

    ret.pop_back(); // remove last '/'

    return ret;
}
