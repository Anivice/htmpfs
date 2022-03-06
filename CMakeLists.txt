cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME "htmpfs")
set(PROJECT_VERSION "0.0.1")

project(${PROJECT_NAME})

set(EXTERNAL_LIBRARIES "")

set(CMAKE_CXX_STANDARD 20)

include(modules/func_def.cmake)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_definitions(CMAKE_BUILD_DEBUG=1)
    enable_testing()
endif()

add_compile_definitions("_FILE_OFFSET_BITS=64")
add_compile_definitions("PACKAGE_NAME=\"${PROJECT_NAME}\"")
add_compile_definitions("PACKAGE_VERSION=\"${PROJECT_VERSION}\"")

# default definitions
string(TOUPPER ${PROJECT_NAME} MARCO_PREFIX)
default_definition(${MARCO_PREFIX}_PREFIX "\"(${PROJECT_NAME}) \"")

# add external library
#set(SHA256SUM_SRC_FILES "")
#set(SHA256SUM_SRC_FILE_NO_PREFIX sha256sum.cpp)
#generate_ext_lib_files(${SHA256SUM_SRC_FILE_NO_PREFIX}
#        sha256sum SHA256SUM_SRC_FILES)
#add_ext_lib(sha256sum ${SHA256SUM_SRC_FILES})

# STEMPFS Static Library
add_library(${PROJECT_NAME}
        # directory resolver
        src/directory_resolver.cpp src/include/directory_resolver.h

        # uni htmpfs
        src/include/htmpfs.h src/htmpfs.cpp

        # buffer
        src/buffer_t.cpp src/include/buffer_t.h

        # pathname resolver
        src/path_t.cpp src/include/path_t.h src/include/custom_itr.h

        # Debug:
        src/debug/debug.cpp src/include/debug.h src/debug/error.cpp
        src/include/htmpfs_error.h

        src/uni_utils.cpp src/include/uni_utils.h
        )
target_include_directories(${PROJECT_NAME} PUBLIC src/include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${EXTERNAL_LIBRARIES})

function(add_single_file EXEC_NAME PATH_PREFIX)
    add_executable(${EXEC_NAME} ${PATH_PREFIX}/${EXEC_NAME}.cpp)
    target_include_directories(${EXEC_NAME} PUBLIC src/include)
    target_link_libraries(${EXEC_NAME} PUBLIC ${EXTERNAL_LIBRARIES} ${PROJECT_NAME})

    message("Single exec `${EXEC_NAME}` enabled")
endfunction()

    # Unit tests
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    _add_test(error             "Error test")
    _add_test(debug             "Debug test")
    _add_test(buffer_t          "buffer_t test")
    _add_test(custom_iterator   "Test for custom iterator")
    _add_test(path              "Test for pathname resolver")
    _add_test(sig_inode         "Test for single inode")
    _add_test(dir_resolver      "Test for Directory Resolver")
    _add_test(filesystem_snapshot
             "Test for filesystem snapshot")
    _add_test(filesystem
            "Test for filesystem functionality")
endif()
