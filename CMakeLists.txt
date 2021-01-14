cmake_minimum_required(VERSION 3.16)

project(tofu)
set(CMAKE_CXX_STANDARD 20)

## ====== general

if(WIN32)
    set(LIBRARY_SUFFIX lib)

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
else()
    set(LIBRARY_SUFFIX a)
endif()

## ====== picoquic

list(APPEND QUIC_INCLUDES "${PROJECT_SOURCE_DIR}/libs/picoquic/picoquic/")
list(APPEND QUIC_INCLUDES "${PROJECT_SOURCE_DIR}/libs/picoquic/loglib/")

if(WIN32)
    set(QUIC_LIB_DIR "${PROJECT_SOURCE_DIR}/libs/picoquic/x64/Release/")

    list(APPEND QUIC_LIBS "${QUIC_LIB_DIR}loglib.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${QUIC_LIB_DIR}picohttp.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${QUIC_LIB_DIR}picoquic.${LIBRARY_SUFFIX}")

    set(TLS_LIB_DIR "${PROJECT_SOURCE_DIR}/libs/picotls/picotlsvs/x64/Release/")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}picotls-bcrypt.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}picotls-core.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}picotls-esni.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}picotls-fusion.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}picotls-minicrypto.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}picotls-minicrypto-deps.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}picotls-openssl.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}picotlsvs.${LIBRARY_SUFFIX}")
    # list(APPEND QUIC_LIBS "${TLS_LIB_DIR}ptlsbench.${LIBRARY_SUFFIX}")

    set(OPENSSL64DIR $ENV{OPENSSL64DIR})

    # list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/capi.${LIBRARY_SUFFIX}")
    # list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/dasync.${LIBRARY_SUFFIX}")
    # list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/libapps.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/libcrypto.${LIBRARY_SUFFIX}")
    # list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/libcrypto_static.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/libssl.${LIBRARY_SUFFIX}")
    # list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/libssl_static.${LIBRARY_SUFFIX}")
    # list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/libtestutil.${LIBRARY_SUFFIX}")
    # list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/openssl.${LIBRARY_SUFFIX}")
    # list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/ossltest.${LIBRARY_SUFFIX}")
    # list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/padlock.${LIBRARY_SUFFIX}")
    # list(APPEND QUIC_LIBS "${OPENSSL64DIR}/lib/uitest.${LIBRARY_SUFFIX}")

    list(APPEND QUIC_LIBS Ws2_32.lib)
else()
    set(QUIC_LIB_DIR "${PROJECT_SOURCE_DIR}/libs/picoquic/")

    list(APPEND QUIC_LIBS "${QUIC_LIB_DIR}libpicoquic-log.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${QUIC_LIB_DIR}libpicohttp-core.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${QUIC_LIB_DIR}libpicoquic-core.${LIBRARY_SUFFIX}")

    set(TLS_LIB_DIR "${PROJECT_SOURCE_DIR}/libs/picotls/")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}libpicotls-core.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}libpicotls-fusion.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}libpicotls-minicrypto.${LIBRARY_SUFFIX}")
    list(APPEND QUIC_LIBS "${TLS_LIB_DIR}libpicotls-openssl.${LIBRARY_SUFFIX}")

    list(APPEND QUIC_LIBS crypto)
    list(APPEND QUIC_LIBS ssl)
    list(APPEND QUIC_LIBS pthread)
endif()

## ======

add_subdirectory(libs/fmt)

add_subdirectory(core)
add_subdirectory(core-test)
add_subdirectory(sandbox)
# add_subdirectory(libs/box2d/src)
