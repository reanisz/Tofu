cmake_minimum_required (VERSION 3.10.2)

# =====
file(GLOB_RECURSE source_files RELAITIVE "${CMAKE_CURRENT_LIST_DIR}/src" "*.cpp")
file(GLOB_RECURSE include_files RELAITIVE "${CMAKE_CURRENT_LIST_DIR}/include" "*.h")

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/src/" PREFIX "src" FILES ${source_files})
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/include/" PREFIX "include" FILES ${include_files})

add_executable(tofu_core_test
    ${source_files}
    ${include_files}
    )

# =====

include (CMakeLists-configure-gtest.txt.in)
include_directories("${PROJECT_SOURCE_DIR}/core/include")

target_link_libraries(tofu_core_test gtest_main)
target_link_libraries(tofu_core_test tofu_core)
add_test(NAME tofu_core_test COMMAND tofu_core_test)

enable_testing()
