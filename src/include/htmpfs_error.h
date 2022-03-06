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

/// _ADD_ERROR_INFORMATION_(error_code_name, error_code_value, error_code_text)
#define _ADD_ERROR_INFORMATION_(code, val, info) \
static const char * code##_ERR_TEXT = HTMPFS_PREFIX info; \
const unsigned long int code = val;


/// Define error information
_ADD_ERROR_INFORMATION_(HTMPFS_SUCCESSFUL,              0x00000000,     "Successful")
_ADD_ERROR_INFORMATION_(HTMPFS_EXT_LIB_ERR,             0xA0000001,     "External library error")
_ADD_ERROR_INFORMATION_(HTMPFS_DOUBLE_ALLOC,            0xA0000002,     "Double allocating in one node")
_ADD_ERROR_INFORMATION_(HTMPFS_ILLEGAL_ACCESS,          0xA0000003,     "Illegal access of memory")
_ADD_ERROR_INFORMATION_(HTMPFS_SNAPSHOT_VER_DEPLETED,   0xA0000004,     "Snapshot version depleted")
_ADD_ERROR_INFORMATION_(HTMPFS_NO_SUCH_SNAPSHOT,        0xA0000005,     "Snapshot not found")
_ADD_ERROR_INFORMATION_(HTMPFS_DOUBLE_SNAPSHOT,         0xA0000006,     "Double snapshot")
_ADD_ERROR_INFORMATION_(HTMPFS_BLOCK_NOT_FOUND,         0xA0000007,     "Block not found")
_ADD_ERROR_INFORMATION_(HTMPFS_BUFFER_ID_DEPLETED,      0xA0000008,     "Buffer ID depleted")
_ADD_ERROR_INFORMATION_(HTMPFS_BUFFER_SHORT_WRITE,      0xA0000009,     "Buffer short write")
_ADD_ERROR_INFORMATION_(HTMPFS_NOT_A_DIRECTORY,         0xA000000A,     "Inode is not a directory")
_ADD_ERROR_INFORMATION_(HTMPFS_DOUBLE_MKPATHNAME,       0xA000000B,     "Pathname already exists")
_ADD_ERROR_INFORMATION_(HTMPFS_NO_SUCH_FILE_OR_DIR,     0xA000000C,     "No such file or directory")
_ADD_ERROR_INFORMATION_(HTMPFS_REQUESTED_BUFFER_NOT_FOUND,  0xA000000D, "Requested buffer not found")
_ADD_ERROR_INFORMATION_(HTMPFS_REQUESTED_INODE_NOT_FOUND,   0xA000000E, "Requested inode not found")
_ADD_ERROR_INFORMATION_(HTMPFS_REQUESTED_VERSION_NOT_FOUND, 0xA000000F, "Requested version not found")
_ADD_ERROR_INFORMATION_(HTMPFS_DIR_NOT_EMPTY,           0xA0000011,     "Directory inode not empty")
_ADD_ERROR_INFORMATION_(HTMPFS_BUFFER_SHORT_READ,       0xA0000012,     "Buffer short read")


/// Filesystem Error Type
class HTMPFS_error_t : public std::exception
{
private:
    uint32_t    error_code;
    error_t     _errno;
    std::string info;

public:
    /// Generate a error with error code
    /** @param _code Your error code
     *  @param _info expanded information for error
     **/
    explicit HTMPFS_error_t(unsigned int _code = 0, std::string _info = "(no specified information)") noexcept
        : error_code(_code), _errno(errno), info(std::move(_info))
        {
            std::cerr << "HTMPFS Error thrown" << std::endl;
            _output_error_message();
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
