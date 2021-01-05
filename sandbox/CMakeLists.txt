cmake_minimum_required(VERSION 3.16)

## ====
file(GLOB source_files "*.cpp")
add_executable(tofu_sandbox ${source_files})
MESSAGE(STATUS "sandbox sources: ${source_files}")

## ==== core
LIST(APPEND LIBS tofu_core)
include_directories(../core/include/)

## ==== fmt
LIST(APPEND LIBS fmt)
include_directories(../libs/fmt/include/)

## ==== msquic
set(QUIC_LIB_DIR ${PROJECT_SOURCE_DIR}/libs/msquic/build/linux/x64_openssl/obj/Debug)
set(QUIC_OPENSSL_LIB_DIR ${PROJECT_SOURCE_DIR}/libs/msquic/build/linux/x64_openssl/openssl/lib)

file(GLOB QUIC_LIBS "${QUIC_LIB_DIR}/*.a")

LIST(APPEND LIBS ${QUIC_LIBS})
MESSAGE(STATUS "QUIC_LIBS: ${QUIC_LIBS}")

# LIST(APPEND LIBS ${QUIC_LIB_DIR}/libcore.a)
# LIST(APPEND LIBS ${QUIC_LIB_DIR}/libperflib.a)
# LIST(APPEND LIBS ${QUIC_LIB_DIR}/libplatform.a)
# LIST(APPEND LIBS ${QUIC_LIB_DIR}/libtestlib.a)
# 
# LIST(APPEND LIBS ${QUIC_LIB_DIR}/libcore.clog.a)
# LIST(APPEND LIBS ${QUIC_LIB_DIR}/libinterop.clog.a)

LIST(APPEND LIBS ${QUIC_OPENSSL_LIB_DIR}/libssl.a)
LIST(APPEND LIBS ${QUIC_OPENSSL_LIB_DIR}/libcrypto.a)

LIST(APPEND LIBS pthread)
LIST(APPEND LIBS dl)

# libs/msquic/build/linux/x64_openssl/lib/libgtestd.a
# libs/msquic/build/linux/x64_openssl/lib/libgtest_maind.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libcore.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libcore.clog.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libinterop.clog.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libmsquiccoretest.clog.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libmsquicplatformtest.clog.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libmsquictest.clog.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libperfbin.clog.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libperflib.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libperflib.clog.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libplatform.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libplatform.clog.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libtestlib.a
# libs/msquic/build/linux/x64_openssl/obj/Debug/libtestlib.clog.a
# libs/msquic/build/linux/x64_openssl/openssl/lib/libcrypto.a
# libs/msquic/build/linux/x64_openssl/openssl/lib/libssl.a
# libs/msquic/build/linux/x64_openssl/submodules/openssl/apps/libapps.a
# libs/msquic/build/linux/x64_openssl/submodules/openssl/libcrypto.a
# libs/msquic/build/linux/x64_openssl/submodules/openssl/libssl.a
# libs/msquic/build/linux/x64_openssl/submodules/openssl/providers/libcommon.a
# libs/msquic/build/linux/x64_openssl/submodules/openssl/providers/libimplementations.a
# libs/msquic/build/linux/x64_openssl/submodules/openssl/providers/liblegacy.a
# libs/msquic/build/linux/x64_openssl/submodules/openssl/providers/libnonfips.a


include_directories(../libs/msquic/src/inc/)

## ===========
target_link_libraries(tofu_sandbox ${LIBS})
