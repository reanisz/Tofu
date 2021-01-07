cmake_minimum_required(VERSION 3.16)

project(tofu)
set(CMAKE_CXX_STANDARD 20)

## ===== msquic

if(WIN32)
    set(LIBRARY_SUFFIX lib)
    set(QUIC_PLATFORM windows)
    set(QUIC_SSL_LIB x64_schannel)
else()
    set(LIBRARY_SUFFIX a)
    set(QUIC_PLATFORM linux)
    set(QUIC_SSL_LIB x64_openssl)

    set(QUIC_SSL_LIB_DIR ${PROJECT_SOURCE_DIR}/libs/msquic/build/${QUIC_PLATFORM}/${QUIC_SSL_LIB}/openssl/lib)
endif()

set(QUIC_LIB_DIR ${PROJECT_SOURCE_DIR}/libs/msquic/build/${QUIC_PLATFORM}/${QUIC_SSL_LIB}/obj/Debug)

if(QUIC_SSL_LIB_DIR)
    file(GLOB QUIC_SSL_LIBS "${QUIC_OPENSSL_LIB_DIR}/*.${LIBRARY_SUFFIX}")
    LIST(APPEND QUIC_LIBS ${QUIC_SSL_LIBS})
    MESSAGE(STATUS "QUIC_SSL_LIBS: ${QUIC_SSL_LIBS}")
endif()

file(GLOB QUIC_CORE_LIBS "${QUIC_LIB_DIR}/*.${LIBRARY_SUFFIX}")

LIST(APPEND QUIC_LIBS ${QUIC_CORE_LIBS})

if(WIN32)
    LIST(APPEND QUIC_LIBS ntdll.lib)
    LIST(APPEND QUIC_LIBS bcrypt.lib)
    LIST(APPEND QUIC_LIBS ncrypt.lib)
    LIST(APPEND QUIC_LIBS ws2_32.lib)
    LIST(APPEND QUIC_LIBS Crypt32.lib)
    LIST(APPEND QUIC_LIBS Secur32.lib)
    LIST(APPEND QUIC_LIBS iphlpapi.dll)
else()
    LIST(APPEND QUIC_LIBS pthread)
    LIST(APPEND QUIC_LIBS dl)
endif()

## ======================

add_subdirectory(libs/fmt)

add_subdirectory(core)
add_subdirectory(core-test)
add_subdirectory(sandbox)

# add_subdirectory(libs/box2d/src)
