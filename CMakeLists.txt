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

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBFUSE REQUIRED fuse)

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
        src/htmpfs/directory_resolver.cpp src/include/htmpfs/directory_resolver.h

        # uni htmpfs
        src/include/htmpfs/htmpfs.h src/htmpfs/htmpfs.cpp

        # buffer
        src/htmpfs/buffer_t.cpp src/include/htmpfs/buffer_t.h

        # pathname resolver
        src/htmpfs/path_t.cpp src/include/htmpfs/path_t.h

        # low-level I/O
        src/disk_ops/ll_io.cpp src/include/ll_io/ll_io.h

        # bitmap utility
        src/htmpfs/bitmap_t.cpp src/include/htmpfs/bitmap_t.h

        # Debug:
        src/debug/debug.cpp src/include/debug.h src/debug/error.cpp
        src/include/htmpfs_error.h

        src/utils/uni_utils.cpp src/include/uni_utils.h
        )
target_include_directories(${PROJECT_NAME} PUBLIC src/include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${EXTERNAL_LIBRARIES})

add_executable(mount.htmpfs
        src/utils/mount.htmpfs.cpp
        src/utils/fuse.cpp
        src/include/fuse_ops.h)
target_include_directories(mount.htmpfs PUBLIC src/include)
target_link_libraries(mount.htmpfs PUBLIC ${EXTERNAL_LIBRARIES} ${PROJECT_NAME})
target_link_libraries(mount.htmpfs PUBLIC ${LIBFUSE_LIBRARIES})
target_include_directories(mount.htmpfs PUBLIC ${LIBFUSE_INCLUDE_DIRS})
target_compile_options(mount.htmpfs PUBLIC ${LIBFUSE_CFLAGS_OTHER})

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
    _add_test(path              "Test for pathname resolver")
    _add_test(sig_inode         "Test for single inode")
    _add_test(dir_resolver      "Test for Directory Resolver")
    _add_test(filesystem_snapshot
             "Test for filesystem snapshot")
    _add_test(filesystem
            "Test for filesystem functionality")
    _add_test(sig_inode_snapshot "Test for single inode snapshot I/O")
    _add_test(ll_io             "Test for direct I/O support")
    _add_test(bitmap            "Test for bitmap management support")
endif()
