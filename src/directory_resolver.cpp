#include <directory_resolver.h>
#include <htmpfs_error.h>
#include <htmpfs.h>
#include <debug.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

template < typename Type >
inline std::string type_to_string(Type num)
{
    std::string ret;
    char * n_str = (char*)&num;
    for (uint64_t i = 0; i < sizeof(num); i++)
    {
        ret += n_str[i];
    }

    return ret;
}

template < typename Type >
inline Type string_to_type(const std::string & str)
{
    Type ret = *((Type*)str.c_str());
    return ret;
}

template < typename Type >
inline void delete_elem_of_str_at_head(std::string & str)
{
    for (uint64_t i = 0; i < sizeof(Type); i++)
    {
        if (!str.empty()) {
            str.erase(str.begin());
        }
    }
}

directory_resolver_t::directory_resolver_t(inode_t *_associated_inode, uint64_t ver)
{
    if (!_associated_inode->__is_dentry())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_NOT_A_DIRECTORY);
    }

    associated_inode = _associated_inode;
    access_version = ver;
    refresh();
}

void directory_resolver_t::refresh()
{
    path.clear();

    if (associated_inode->current_data_size(access_version) == 0)
    {
        return;
    }

    std::string name, inode_id;
    std::string all_path = associated_inode->to_string(access_version);
    char name_buff[129] { };

    // ignore empty dentry
    if (all_path.empty())
    {
        return;
    }

    // get save pack count, which is at the head of the string
    auto save_pack_count = string_to_type<uint64_t>(all_path);
    delete_elem_of_str_at_head<uint64_t>(all_path);

    for (uint64_t i = 0; i < save_pack_count; i++)
    {
        flat_path_pack_t save_pack;
        path_pack_t runtime_path_pack;

        save_pack = string_to_type<flat_path_pack_t>(all_path);
        delete_elem_of_str_at_head<flat_path_pack_t>(all_path);
        strcpy(name_buff, save_pack.pathname);
        runtime_path_pack.pathname = name_buff;
        runtime_path_pack.inode_id = save_pack.inode_id;

        path.emplace_back(runtime_path_pack);
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
        if (!memcmp(i.pathname.c_str(), pathname.c_str(),
                    MIN(pathname.length(), i.pathname.length())))
        {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_DOUBLE_MKPATHNAME);
        }
    }

    path.emplace_back(path_pack_t { .pathname = pathname, .inode_id = inode_id} );
}

uint64_t directory_resolver_t::namei(const std::string & pathname)
{
    for (const auto& i : path)
    {
        if (!memcmp(i.pathname.c_str(), pathname.c_str(),
                    MIN(i.pathname.length(), pathname.length())))
        {
            return i.inode_id;
        }
    }

    THROW_HTMPFS_ERROR_STDERR(HTMPFS_NO_SUCH_FILE_OR_DIR);
}

void directory_resolver_t::save_current()
{
    std::string ret;
    ret += type_to_string<uint64_t>(path.size());

    for (const auto& i : path)
    {
        flat_path_pack_t save_pack;
        memcpy(save_pack.pathname,
               i.pathname.c_str(),
               MIN ( sizeof(save_pack.pathname), i.pathname.size() )
               );
        save_pack.inode_id = i.inode_id;

        ret += type_to_string(save_pack);
    }

    associated_inode->write(ret.c_str(), ret.length(), 0,
                            true, __dentry_only(true));
}

bool directory_resolver_t::check_availability(const std::string &pathname)
{
    for (const auto& i : path)
    {
        if (!memcmp(i.pathname.c_str(), pathname.c_str(),
                    MIN(i.pathname.length(), pathname.length())))
        {
            return false;
        }
    }

    return true;
}

void directory_resolver_t::remove_path(const std::string &pathname)
{
    for (auto i = path.begin(); i != path.end(); i++)
    {
        if (!memcmp(i->pathname.c_str(), pathname.c_str(),
                    MIN(i->pathname.length(), pathname.length())))
        {
            path.erase(i);
            return;
        }
    }

    THROW_HTMPFS_ERROR_STDERR(HTMPFS_REQUESTED_INODE_NOT_FOUND,
                              "No dentry matching provided name under current parent inode");
}
