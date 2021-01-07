#pragma once

#include <msquic.h>


namespace quic {

    extern const QUIC_API_TABLE* MsQuic;
    extern const QUIC_REGISTRATION_CONFIG RegConfig;

    extern const QUIC_BUFFER _alpn;

    extern HQUIC _registration;
    extern HQUIC _configuration;

    void init();
#ifdef WIN32 
    void init_configuration(HQUIC& registration, HQUIC& configuration, bool is_server, const char* hash_file);
#else
    void init_configuration(HQUIC& registration, HQUIC& configuration, bool is_server, const char* cert, const char* private_key);
#endif
}

