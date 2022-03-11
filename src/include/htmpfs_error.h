#ifndef HTMPFS_ERROR_H
#define HTMPFS_ERROR_H

/** @file
 * This file defines runtime error class and relevant operations
 */

#include <string>
#include <utility>
#include <cstring>
#include <cstdlib>
#include <debug.h>
#include <cerrno>

/// _ADD_ERROR_INFORMATION_(error_code_name, error_code_value, error_code_text)
#define _ADD_ERROR_INFORMATION_(code, val, info, _errno)    \
static const char * code##_ERR_TEXT = HTMPFS_PREFIX info;   \
const unsigned long int code = val;                         \
const unsigned int code##_ERRNO_CODE = _errno;

/// Define error information
_ADD_ERROR_INFORMATION_(HTMPFS_SUCCESSFUL,              0x00000000,     "Successful",                   0)
_ADD_ERROR_INFORMATION_(HTMPFS_EXT_LIB_ERR,             0xA0000001,     "External library error",       1)
_ADD_ERROR_INFORMATION_(HTMPFS_DOUBLE_ALLOC,            0xA0000002,     "Double allocating in one node", EEXIST)
_ADD_ERROR_INFORMATION_(HTMPFS_ILLEGAL_ACCESS,          0xA0000003,     "Illegal access of memory",     1)
_ADD_ERROR_INFORMATION_(HTMPFS_SNAPSHOT_VER_DEPLETED,   0xA0000004,     "Snapshot version depleted",    1)
_ADD_ERROR_INFORMATION_(HTMPFS_NO_SUCH_SNAPSHOT,        0xA0000005,     "Snapshot not found",           ENOENT)
_ADD_ERROR_INFORMATION_(HTMPFS_DOUBLE_SNAPSHOT,         0xA0000006,     "Double snapshot",              EEXIST)
_ADD_ERROR_INFORMATION_(HTMPFS_BLOCK_NOT_FOUND,         0xA0000007,     "Block not found",              1)
_ADD_ERROR_INFORMATION_(HTMPFS_BUFFER_ID_DEPLETED,      0xA0000008,     "Buffer ID depleted",           1)
_ADD_ERROR_INFORMATION_(HTMPFS_BUFFER_SHORT_OPS,        0xA0000009,     "Buffer short operation",       1)
_ADD_ERROR_INFORMATION_(HTMPFS_NOT_A_DIRECTORY,         0xA000000A,     "Inode is not a directory",     ENOTDIR)
_ADD_ERROR_INFORMATION_(HTMPFS_DOUBLE_MKPATHNAME,       0xA000000B,     "Pathname already exists",      EEXIST)
_ADD_ERROR_INFORMATION_(HTMPFS_NO_SUCH_FILE_OR_DIR,     0xA000000C,     "No such file or directory",    ENOENT)
_ADD_ERROR_INFORMATION_(HTMPFS_REQUESTED_BUFFER_NOT_FOUND,  0xA000000D, "Requested buffer not found",   1)
_ADD_ERROR_INFORMATION_(HTMPFS_REQUESTED_INODE_NOT_FOUND,   0xA000000E, "Requested inode not found",    1)
_ADD_ERROR_INFORMATION_(HTMPFS_DIR_NOT_EMPTY,           0xA0000011,     "Directory inode not empty",    ENOTEMPTY)
_ADD_ERROR_INFORMATION_(HTMPFS_INVALID_DENTRY_NAME,     0xA0000013,     "Invalid dentry name",          EINVAL)
_ADD_ERROR_INFORMATION_(HTMPFS_INVALID_WRITE_INVOKE,    0xA0000014,     "Invalid write invoke",         1)
_ADD_ERROR_INFORMATION_(HTMPFS_INVALID_READ_INVOKE,     0xA0000015,     "Invalid read invoke",          1)
_ADD_ERROR_INFORMATION_(HTMPFS_CANNOT_REMOVE_ROOT,      0xA0000016,     "Cannot remove root inode",     EISDIR)
_ADD_ERROR_INFORMATION_(HTMPFS_CANNOT_OPEN_DEVICE,      0xA0000017,     "Cannot open device",           1)
_ADD_ERROR_INFORMATION_(HTMPFS_CANNOT_LSEEK_DEVICE,     0xA0000018,     "Cannot lseek device",          1)
_ADD_ERROR_INFORMATION_(HTMPFS_MEET_DEVICE_BOUNDARY,    0xA0000019,     "Meet device boundary",         1)
_ADD_ERROR_INFORMATION_(HTMPFS_BLOCK_SHORT_OPS,         0xA000001A,     "Block short I/O operation",    1)

/// Filesystem Error Type
class HTMPFS_error_t : public std::exception
{
private:
    uint32_t    error_code;
    error_t     _errno = 0;
    std::string info;

public:
    /// fill out errno information based on error code
    void fill_out_errno();

    /// Generate a error with error code
    /** @param _code Your error code
     *  @param _info expanded information for error
     **/
    explicit HTMPFS_error_t(unsigned int _code = 0, std::string _info = "(no specified information)") noexcept
        : info(std::move(_info))
        {
            error_code = _code;
#ifdef CMAKE_BUILD_DEBUG
            if (!__disable_output)
            {
                std::cerr << "HTMPFS Error thrown" << std::endl;
                _output_error_message();
            }
#endif // CMAKE_BUILD_DEBUG

            fill_out_errno();
        }

    /// Return explanation of current error
    [[nodiscard]] const char * what() const noexcept override;

    /// Return the explanation of errno snapshoted when the current error is generated
    [[nodiscard]] const char * what_errno() const noexcept { return strerror(_errno); };

    /// Return the errno snapshoted when the current error is generated
    [[nodiscard]] error_t my_errno() const noexcept { return _errno; }

    /// Return the error code of current error
    [[nodiscard]] unsigned int my_errcode() const noexcept { return error_code; }

private:
    /// output error message
    /// @param msg system message
    void _output_error_message() const noexcept;
};

/// this section is defining a marco with variable parameter count
#define _THROW_HTMPFS_2(_code, _info)  throw HTMPFS_error_t(_code, _info)
#define _THROW_HTMPFS_1(_code)         throw HTMPFS_error_t(_code)
#define _THROW_HTMPFS_0()              throw HTMPFS_error_t()

#define _FUNC_CHOOSER(_f1, _f2, _f3, ...) _f3
#define _FUNC_RECOMPOSER(argsWithParentheses) _FUNC_CHOOSER argsWithParentheses
#define _CHOOSE_FROM_ARG_COUNT(...) _FUNC_RECOMPOSER((__VA_ARGS__, _THROW_HTMPFS_2, _THROW_HTMPFS_1, ))
#define _NO_ARG_EXPANDER() ,,_THROW_HTMPFS_0
#define _MACRO_CHOOSER(...) _CHOOSE_FROM_ARG_COUNT(_NO_ARG_EXPANDER __VA_ARGS__ ())

/// generate error using HTMPFS_error_t
/// THROW_HTMPFS_ERROR(output_pipe, code_name, additional_information)
/// THROW_HTMPFS_ERROR(output_pipe, code_name)
/// THROW_HTMPFS_ERROR(output_pipe) < snapshot errno only
/// output_pipe == 1: stdout, output_pipe == 2, stderr
/// other value will be ignored and pipe will be default to stdout
#define THROW_HTMPFS_ERROR(fd, ...)                             \
            FUNCTION_INFO(fd);                                  \
            OBTAIN_STACK_FRAME(fd);                             \
            _MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
/// generate error using HTMPFS_error_t, output to stderr
#define THROW_HTMPFS_ERROR_STDERR(...)                          \
            FUNCTION_INFO(2);                                   \
            OBTAIN_STACK_FRAME(2);                              \
            _MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#endif //HTMPFS_ERROR_H
