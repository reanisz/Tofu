cmake_minimum_required(VERSION 3.16)

project(tofu_sandbox)

## ====
file(GLOB source_files "*.cpp")
add_executable(tofu_sandbox ${source_files})

include_directories(../core/include/)
target_link_libraries(tofu_core)
