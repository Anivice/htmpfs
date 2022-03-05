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
_ADD_ERROR_INFORMATION_(HTMPFS_SUCCESSFUL,  0x00000000,  "Successful")
_ADD_ERROR_INFORMATION_(HTMPFS_EXT_LIB_ERR, 0xA0000001, "External library error");


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
            _output_error_message(std::to_string(my_errcode()).c_str());
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
    /// @return pointer given by parameter msg
    [[nodiscard]] const char * _output_error_message(const char * msg) const noexcept;
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
