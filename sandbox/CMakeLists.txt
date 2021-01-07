cmake_minimum_required(VERSION 3.16)

## ====
file(GLOB source_files "*.cpp")
add_executable(tofu_sandbox ${source_files})
MESSAGE(STATUS "sandbox sources: ${source_files}")

unset(LIBS)

## ==== core
LIST(APPEND LIBS tofu_core)
include_directories(../core/include/)

## ==== fmt
LIST(APPEND LIBS fmt)
include_directories(../libs/fmt/include/)

## ==== msquic
LIST(APPEND LIBS ${QUIC_LIBS})


include_directories(../libs/msquic/src/inc/)

## ===========
target_link_libraries(tofu_sandbox ${LIBS})
