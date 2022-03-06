#include <directory_resolver.h>
#include <algorithm>
#include <htmpfs_error.h>
#include <htmpfs.h>

directory_resolver_t::directory_resolver_t(inode_t *_associated_inode, uint64_t ver)
: associated_inode(_associated_inode), access_version(ver)
{
    std::string name, inode_id;
    std::string all_path = _associated_inode->to_string(ver);
    bool st_name = false, st_id = true;

    for (auto i : all_path)
    {
        if (i == '/')
        {
            // read stops here
            if (!st_name && st_id && !name.empty())
            {
                uint64_t encoded_inode_id = (*(uint64_t*)inode_id.c_str());

                path.emplace_back(
                        path_pack_t {
                            .pathname = name,
                            .inode_id = encoded_inode_id
                        }
                );

                name.clear();
                inode_id.clear();
            }

            st_name = !st_name;
            st_id = !st_id;
            continue;
        }

        if (st_name)
        {
            name += i;
        }

        if (st_id)
        {
            inode_id += i;
        }
    }
}

std::vector < directory_resolver_t::path_pack_t > directory_resolver_t::to_vector()
{
    return path;
}

void directory_resolver_t::add_path(const std::string & pathname, uint64_t inode_id)
{
    for (const auto& i : path)
    {
        if (i.pathname == pathname) {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_DOUBLE_MKPATHNAME);
        }
    }

    path.emplace_back(path_pack_t { .pathname = pathname, .inode_id = inode_id} );
}

uint64_t directory_resolver_t::namei(const std::string & pathname)
{
    for (const auto& i : path)
    {
        if (i.pathname == pathname) {
            return i.inode_id;
        }
    }

    THROW_HTMPFS_ERROR_STDERR(HTMPFS_NO_SUCH_FILE_OR_DIR);
}

void directory_resolver_t::save_current()
{
    std::string ret;
    ret += "/";

    for (const auto& i : path)
    {
        ret += i.pathname + "/";
        const char * str_inode = (char*)&i.inode_id;
        for (int off = 0; off < 7; off++)
        {
            ret += str_inode[off];
        }

        ret += "/";
    }

    associated_inode->write(ret.c_str(), ret.length(), 0);
}
