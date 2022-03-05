cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME "htmpfs")
set(PROJECT_VERSION "0.0.1")

project(${PROJECT_NAME})

set(EXTERNAL_LIBRARIES "")

set(CMAKE_CXX_STANDARD 20)

include(modules/func_def.cmake)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_definitions(CMAKE_BUILD_DEBUG=1)
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
        src/include/htmpfs_error.h

        # Debug:
        src/debug/debug.cpp src/include/debug.h src/debug/error.cpp
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
    _add_test(error       "Error test")
    _add_test(debug       "Debug test")
endif()
