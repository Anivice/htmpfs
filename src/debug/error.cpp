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
        ERROR_SWITCH_CASE(HTMPFS_DIR_NOT_EMPTY);
        ERROR_SWITCH_CASE(HTMPFS_BUFFER_SHORT_READ);
        ERROR_SWITCH_CASE(HTMPFS_INVALID_DENTRY_NAME);
        ERROR_SWITCH_CASE(HTMPFS_INVALID_WRITE_INVOKE);
        ERROR_SWITCH_CASE(HTMPFS_INVALID_READ_INVOKE);
        ERROR_SWITCH_CASE(HTMPFS_CANNOT_REMOVE_ROOT);
    ERROR_SWITCH_END;
}

void HTMPFS_error_t::_output_error_message() const noexcept
{
    std::cerr <<
            "HTMPFS error: 0x" << std::uppercase << std::hex << error_code << " (" << what() << ")" << "\n" <<
            "System error: " << what_errno() << "\n" <<
            "User specified information: " << info << "\n" << std::flush;
}
