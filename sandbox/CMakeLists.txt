cmake_minimum_required(VERSION 3.16)

## ====
file(GLOB source_files "*.cpp")
add_executable(tofu_sandbox ${source_files})

include_directories("${PROJECT_SOURCE_DIR}/core/include")
include_directories(${QUIC_INCLUDES})

target_link_libraries(tofu_sandbox tofu_core)
target_link_libraries(tofu_sandbox fmt)
target_link_libraries(tofu_sandbox ${QUIC_LIBS})

