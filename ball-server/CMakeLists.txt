cmake_minimum_required(VERSION 3.16)

## ====
file(GLOB_RECURSE source_files RELAITIVE "${CMAKE_CURRENT_LIST_DIR}/src" "*.cpp")
file(GLOB_RECURSE include_files RELAITIVE "${CMAKE_CURRENT_LIST_DIR}/include" "*.h")

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/src/" PREFIX "src" FILES ${source_files})
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/include/" PREFIX "include" FILES ${include_files})

add_executable(ball_server
    ${source_files}
    ${include_files}
    )

## ====

include_directories("include")
include_directories("${PROJECT_SOURCE_DIR}/ball-core/include")
include_directories("${PROJECT_SOURCE_DIR}/libs/entt/src")
include_directories("${PROJECT_SOURCE_DIR}/core/include")
include_directories("${PROJECT_SOURCE_DIR}/quic/include")
include_directories(${QUIC_INCLUDES})
include_directories(${BOX2D_INCLUDES})

target_link_libraries(ball_server ball_core)
target_link_libraries(ball_server tofu_quic)
target_link_libraries(ball_server tofu_core)
target_link_libraries(ball_server fmt)
target_link_libraries(ball_server ${QUIC_LIBS})
target_link_libraries(ball_server ${BOX2D_LIBS})
