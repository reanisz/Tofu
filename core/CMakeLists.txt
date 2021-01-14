cmake_minimum_required(VERSION 3.16)

project(tofu_core)

## ====
set(CMAKE_CXX_STANDARD 20)

include_directories(./include/)

## ====
file(GLOB_RECURSE source_files RELAITIVE "${CMAKE_CURRENT_LIST_DIR}/src" "*.cpp")
file(GLOB_RECURSE include_files RELAITIVE "${CMAKE_CURRENT_LIST_DIR}/include" "*.h")

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/src/" PREFIX "src" FILES ${source_files})
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/include/" PREFIX "include" FILES ${include_files})

add_library(tofu_core
    ${source_files}
    ${include_files}
    )

target_compile_options(tofu_core PUBLIC -fconcepts)

## ==== box2d
# target_link_libraries(tofu_core box2d)
# link_directories(../libs/box2d/)
include_directories(../libs/box2d/include/)
include_directories(../libs/entt/src/)

