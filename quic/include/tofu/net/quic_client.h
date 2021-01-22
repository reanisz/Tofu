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
        ~QuicClient();

        void Start();
        void Exit();

        std::shared_ptr<QuicConnection> GetConnection();

        void OnCloseConnection(const std::shared_ptr<QuicConnection>& connection);

        int CallbackPacketLoop(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx);

        bool HasError() const { return _error.has_value(); }
        const Error& GetError() const { return _error; }

        const QuicClientConfig& GetConfig() const noexcept
        {
            return _config;
        }

    private:
        QuicClientConfig _config;
        Error _error;
        std::atomic<int> _loopReturnCode;

        picoquic_quic_t* _quic = nullptr;
        std::shared_ptr<QuicConnection> _connection;

        std::thread _thread;
        std::atomic<bool> _end = false;
    };
}
