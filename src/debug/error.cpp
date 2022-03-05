#include <htmpfs_error.h>

/** @file
 *  this file implements functions for HTMPFS_error_t
 */

/// used for HTMPFS_error_t::what()
#define ERROR_SWITCH_START \
    switch (error_code) { case 0 : return HTMPFS_SUCCESSFUL_ERR_TEXT
/// used for HTMPFS_error_t::what()
#define ERROR_SWITCH_CASE(val_name) case val_name : return val_name##_ERR_TEXT
/// used for HTMPFS_error_t::what()
#define ERROR_SWITCH_END } __asm__("nop")

const char *HTMPFS_error_t::what() const noexcept 
{
    ERROR_SWITCH_START;
        ERROR_SWITCH_CASE(HTMPFS_EXT_LIB_ERR);
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
