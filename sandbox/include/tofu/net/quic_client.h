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
    class QuicClient;

    class QuicClientConnection
    {
    public:
        QuicClientConnection(observer_ptr<QuicClient> client);
        int CallbackConnection(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);

    private:
        observer_ptr<QuicClient> _client;
        Error _error;
    };

    class QuicClient
    {
    public:
        QuicClient(const QuicClientConfig& config);

        void Start();

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
