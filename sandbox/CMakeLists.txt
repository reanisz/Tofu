cmake_minimum_required(VERSION 3.16)

## ====
file(GLOB_RECURSE source_files RELAITIVE "${CMAKE_CURRENT_LIST_DIR}/src" "*.cpp")
file(GLOB_RECURSE include_files RELAITIVE "${CMAKE_CURRENT_LIST_DIR}/include" "*.h")

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/src/" PREFIX "src" FILES ${source_files})
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/include/" PREFIX "include" FILES ${include_files})

add_executable(tofu_sandbox
    ${source_files}
    ${include_files}
    )

## ====

include_directories("include")
include_directories("${PROJECT_SOURCE_DIR}/core/include")
include_directories("${PROJECT_SOURCE_DIR}/quic/include")
include_directories(${QUIC_INCLUDES})

target_link_libraries(tofu_sandbox tofu_quic)
target_link_libraries(tofu_sandbox tofu_core)
target_link_libraries(tofu_sandbox fmt)
target_link_libraries(tofu_sandbox ${QUIC_LIBS})

