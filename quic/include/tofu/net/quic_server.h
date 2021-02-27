#pragma once

#include <set>
#include <functional>

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
        using callback_on_connect_t = std::function<void(const std::shared_ptr<QuicConnection>&)>;
        using callback_on_close_t = std::function<void(const std::shared_ptr<QuicConnection>&)>;

        QuicServer(const QuicServerConfig& config);
        ~QuicServer();

        void Start();
        void Exit();

        template<class T>
        void ForeachConnections(const T& func)
        {
            std::set<std::shared_ptr<QuicConnection>> connections;
            {
                std::lock_guard lock{ _mutexConnections };
                connections = _connections;
            }
            for (auto& cnx : connections)
            {
                func(*cnx);
            }
        }

        void OnCloseConnection(const std::shared_ptr<QuicConnection>& connection);

        int CallbackConnectionInit(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);
        int CallbackPacketLoop(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx);

        bool HasError() const { return _error.has_value(); }
        const Error& GetError() const { return _error; }

        const QuicServerConfig& GetConfig() const noexcept
        {
            return _config;
        }

        // === callbacks
        void SetCallbackOnConnect(const callback_on_connect_t& function)
        {
            _callbackOnConnect = function;
        }
        void SetCallbackOnClose(const callback_on_close_t& function)
        {
            _callbackOnClose = function;
        }

    private:
        QuicServerConfig _config;
        Error _error;

        picoquic_quic_t* _quic = nullptr;

        std::thread _thread;
        std::atomic<bool> _end = false;
        std::atomic<int> _loopReturnCode;

        std::mutex _mutexConnections;
        std::set<std::shared_ptr<QuicConnection>> _connections;

        // === callbacks
        callback_on_connect_t _callbackOnConnect;
        callback_on_close_t _callbackOnClose;
    };
}
