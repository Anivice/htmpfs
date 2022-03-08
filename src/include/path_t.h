#ifndef HTMPFS_PATH_T_H
#define HTMPFS_PATH_T_H

/** @file
 *  this file implements functions for path finder
 */

#include <string>
#include <vector>
#include <buffer_t.h>

class path_t
{
public:
    typedef std::vector < std::string >::iterator iterator;

private:
    std::vector < std::string > pathname;

public:
    explicit        path_t  (const std::string &);
    iterator        begin   ()                      { return pathname.begin();      }
    iterator        end     ()                      { return pathname.end();        }
    htmpfs_size_t   size    ()                      { return pathname.size();       }
    iterator        last    ()                      { return (--pathname.end());    }
    std::string     pop_end ()
        {
            std::string ret = * (--pathname.end());
            pathname.pop_back();
            return ret;
        }

    std::string to_string();
};

#endif //HTMPFS_PATH_T_H
