#pragma once

#include <set>

#include "tofu/net/quic.h"

namespace tofu::net
{
    struct QuicServerConfig
    {
        QuicConfig _config;

        Port _port;
        std::filesystem::path _certFile;
        std::filesystem::path _secretFile;
        std::string _alpn;
    };

    class QuicServer
    {
    public:
        QuicServer(const QuicServerConfig& config);

        void Start();

        void OnCloseConnection(const std::shared_ptr<QuicConnection>& connection);

        int CallbackConnectionInit(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);
        int CallbackPacketLoop(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx);

        bool HasError() const { return _error.has_value(); }
        const Error& GetError() const { return _error; }

    private:
        QuicServerConfig _config;
        Error _error;

        std::thread _thread;
        std::atomic<bool> _end = false;
        std::set<std::shared_ptr<QuicConnection>> _connections;
    };
}
