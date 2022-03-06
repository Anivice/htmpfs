#include <directory_resolver.h>
#include <algorithm>
#include <htmpfs_error.h>

directory_resolver_t::directory_resolver_t(buffer_t & _path, inode_t *_associated_inode)
: associated_inode(_associated_inode)
{
    std::string name, inode_id;
    std::string all_path = _path.to_string();
    bool st_name = false, st_id = true;

    for (auto i : all_path)
    {
        if (i == '/')
        {
            // read stops here
            if (!st_name && st_id)
            {
                uint64_t encoded_inode_id = 0;

                for (uint64_t off = inode_id.length(); off > 0; off--)
                {
                    // if length == 0, this operation will be ignored
                    // so it's safe here to use off - 1
                    uint64_t bit_cmp = (uint64_t)i << (uint64_t)((off - 1) * 8);

                    encoded_inode_id |= bit_cmp;
                }

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

std::vector < char > directory_resolver_t::to_string()
{
    std::vector <char> ret;
    ret.emplace_back('/');

    for (const auto& i : path)
    {
        for (const auto & path_i : i.pathname) {
            ret.emplace_back(path_i);
        }

        for (int off = 7; off >= 0; off--) {
            ret.emplace_back(((uint64_t) i.inode_id) >> (uint64_t)off);
        }
    }

    return ret;
}
