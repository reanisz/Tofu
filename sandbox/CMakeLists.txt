cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

## ====
file(GLOB source_files "*.cpp")
add_executable(tofu_sandbox ${source_files})

include_directories(../core/include/)
target_link_libraries(tofu_core)

include_directories(../libs/lsquic/include/)

## ==== link

# libboringssl
LIST(APPEND LIBS crypto)
LIST(APPEND LIBS ssl)
LIST(APPEND LIBS decrepit)

# liblsquic
LIST(APPEND LIBS ${PROJECT_SOURCE_DIR}/libs-build/lsquic-${PLATFORM_SUFFIX}/src/liblsquic/liblsquic.a)

# zlib
LIST(APPEND LIBS ${ZLIB_LIB})

target_link_libraries(tofu_sandbox ${LIBS})

