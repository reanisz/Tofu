cmake_minimum_required(VERSION 3.16)

project(tofu)

# ======================================

# zlib 
# Find zlib and libevent header files and library files
# TODO: libevent is not strictly necessary to build the library.
FIND_PATH(ZLIB_INCLUDE_DIR NAMES zlib.h)
IF (ZLIB_INCLUDE_DIR)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ELSE()
    MESSAGE(FATAL_ERROR "zlib.h was not found")
ENDIF()
IF (CMAKE_SYSTEM_NAME STREQUAL Windows)
    FIND_LIBRARY(ZLIB_LIB zlib)
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    # XXX somehow FIND_LIBRARY() does not find zlib on Travis?
    SET(ZLIB_LIB z)
ELSE()
    FIND_LIBRARY(ZLIB_LIB libz${LIB_SUFFIX})
ENDIF()
IF(ZLIB_LIB)
    MESSAGE(STATUS "Found zlib: ${ZLIB_LIB}")
ELSE()
    MESSAGE(STATUS "zlib not found")
ENDIF()

# ======================================

add_subdirectory(libs/boringssl/)
# add_subdirectory(libs/lsquic/)

add_subdirectory(core)
add_subdirectory(core-test)
add_subdirectory(sandbox)
# add_subdirectory(libs/box2d/src)
