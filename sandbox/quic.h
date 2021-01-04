#pragma once

#include <msquic.h>


namespace quic {

    extern const QUIC_API_TABLE* MsQuic;
    extern const QUIC_REGISTRATION_CONFIG RegConfig;

    extern const QUIC_BUFFER _alpn;

    extern HQUIC _registration;
    extern HQUIC _configuration;

    void init();
    void init_configuration(HQUIC& registration, HQUIC& configuration, bool is_server, const char* cert, const char* private_key);
}

