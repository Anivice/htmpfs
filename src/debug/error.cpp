#include <htmpfs_error.h>

/** @file
 *  this file implements functions for HTMPFS_error_t
 */

/// used for HTMPFS_error_t::what()
#define ERROR_SWITCH_START \
    switch (error_code) { case HTMPFS_SUCCESSFUL : return HTMPFS_SUCCESSFUL_ERR_TEXT
/// used for HTMPFS_error_t::what()
#define ERROR_SWITCH_CASE(val_name) case val_name : return val_name##_ERR_TEXT
/// used for HTMPFS_error_t::what()
#define ERROR_SWITCH_END default : return "Unknown error"; } __asm__("nop")

const char *HTMPFS_error_t::what() const noexcept 
{
    ERROR_SWITCH_START;
        ERROR_SWITCH_CASE(HTMPFS_EXT_LIB_ERR);
        ERROR_SWITCH_CASE(HTMPFS_DOUBLE_ALLOC);
        ERROR_SWITCH_CASE(HTMPFS_ILLEGAL_ACCESS);
        ERROR_SWITCH_CASE(HTMPFS_SNAPSHOT_VER_DEPLETED);
        ERROR_SWITCH_CASE(HTMPFS_NO_SUCH_SNAPSHOT);
        ERROR_SWITCH_CASE(HTMPFS_DOUBLE_SNAPSHOT);
        ERROR_SWITCH_CASE(HTMPFS_BUFFER_ID_DEPLETED);
        ERROR_SWITCH_CASE(HTMPFS_BUFFER_SHORT_WRITE);
        ERROR_SWITCH_CASE(HTMPFS_NOT_A_DIRECTORY);
        ERROR_SWITCH_CASE(HTMPFS_DOUBLE_MKPATHNAME);
        ERROR_SWITCH_CASE(HTMPFS_NO_SUCH_FILE_OR_DIR);
        ERROR_SWITCH_CASE(HTMPFS_REQUESTED_BUFFER_NOT_FOUND);
        ERROR_SWITCH_CASE(HTMPFS_REQUESTED_INODE_NOT_FOUND);
        ERROR_SWITCH_CASE(HTMPFS_REQUESTED_VERSION_NOT_FOUND);
    ERROR_SWITCH_END;
}

void HTMPFS_error_t::_output_error_message(const char * direct_message) const noexcept
{
    std::string errno_msg = what_errno();
    std::string error_msg =
            "HTMPFS error: " + std::string(direct_message) + "\n" +
            "System error: " + errno_msg + "\n" +
            "User specified information: " + info + "\n";
    std::cerr << error_msg;
}
