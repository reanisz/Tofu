cmake_minimum_required(VERSION 3.16)

add_compile_definitions(TOFU_ENABLE_BOX2D)

## ====
file(GLOB_RECURSE source_files RELAITIVE "${CMAKE_CURRENT_LIST_DIR}/src" "*.cpp")
file(GLOB_RECURSE include_files RELAITIVE "${CMAKE_CURRENT_LIST_DIR}/include" "*.h")

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/src/" PREFIX "src" FILES ${source_files})
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/include/" PREFIX "include" FILES ${include_files})

add_library(ball_core
    ${source_files}
    ${include_files}
    )

## ====

include_directories("include")
include_directories("${PROJECT_SOURCE_DIR}/core/include")
include_directories("${PROJECT_SOURCE_DIR}/quic/include")
include_directories("${PROJECT_SOURCE_DIR}/libs/entt/src")
include_directories("${PROJECT_SOURCE_DIR}/libs/box2d/include")
include_directories(${QUIC_INCLUDES})
include_directories(${BOX2D_INCLUDES})

target_link_libraries(ball_core tofu_quic)
target_link_libraries(ball_core tofu_core)
target_link_libraries(ball_core fmt)
target_link_libraries(ball_core ${QUIC_LIBS})
target_link_libraries(ball_core ${BOX2D_LIBS})

