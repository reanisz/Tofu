#pragma once

#include "quic.h"

namespace tofu::net
{
    struct QuicClientConfig
    {
        QuicConfig _config;

        std::string _serverName;
        Port _port;
        std::string _alpn;
    };
    class QuicClient
    {
    public:
        QuicClient(const QuicClientConfig& config);

        void Start();

        void OnCloseConnection(const std::shared_ptr<QuicConnection>& connection);

        int CallbackPacketLoop(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx);

        bool HasError() const { return _error.has_value(); }
        const Error& GetError() const { return _error; }

    private:
        QuicClientConfig _config;
        Error _error;

        std::thread _thread;
        std::atomic<bool> _end = false;
    };
}
