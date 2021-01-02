cmake_minimum_required(VERSION 3.16)

project(tofu_core)

## ====
set(CMAKE_CXX_STANDARD 20)

include_directories(./include/)

## ====
file(GLOB source_files "**/*.cpp")
add_library(tofu_core ${source_files})

target_compile_options(tofu_core PUBLIC -fconcepts)

## ==== box2d
# target_link_libraries(tofu_core box2d)
# link_directories(../libs/box2d/)
include_directories(../libs/box2d/include/)
include_directories(../libs/entt/src/)

