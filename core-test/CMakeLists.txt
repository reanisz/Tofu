cmake_minimum_required (VERSION 3.10.2)

include (CMakeLists-configure-gtest.txt.in)

include_directories("${PROJECT_SOURCE_DIR}/core/include")

add_executable(tofu_core_test main.cpp)
target_link_libraries(tofu_core_test gtest_main)
target_link_libraries(tofu_core_test tofu_core)
add_test(NAME tofu_core_test COMMAND tofu_core_test)

enable_testing()
