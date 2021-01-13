#include <fmt/core.h>

#include "quic_server.h"

namespace tofu::net
{
    QuicServer::QuicServer(const QuicServerConfig& config)
        : _config(config)
    {
    }

    void QuicServer::Start()
    {
        fmt::print("Starting server on port {}\n", *_config._port);
        auto current_time = picoquic_current_time();

        picoquic_quic_t* quic = picoquic_create(
            8, // nb_connections
            _config._certFile.string().c_str(),
            _config._secretFile.string().c_str(),
            nullptr, // cert_root_file_name
            _config._alpn.c_str(),
            nullptr, // REQUIRE: callback,
            this, // context
            nullptr, // cnx_id_callback
            nullptr, // cnx_id_callback_data
            nullptr, // reset_seed
            current_time, //seed
            nullptr, // p_simulated_time
            nullptr, // ticket_file_name
            nullptr, // ticket_encryption_key
            0 // ticket_encryption_key_length
            );
    }
}

