#include <fmt/core.h>

#include <picoquic.h>
#include <picoquic_utils.h>
#include <picoquic_packet_loop.h>
#include <picosocks.h>
#include <autoqlog.h>

#include "tofu/net/quic_server.h"

namespace tofu::net
{
    QuicServer::QuicServer(const QuicServerConfig& config)
        : _config(config)
        , _end(false)
    {
    }

    QuicServer::~QuicServer()
    {
        Exit();
    }

    void QuicServer::Start()
    {
        fmt::print("[QuicServer] Starting server on port {}\n", *_config._port);
        auto current_time = picoquic_current_time();

        _quic = picoquic_create(
            1, // nb_connections
            _config._certFile.string().c_str(),
            _config._secretFile.string().c_str(),
            nullptr, // cert_root_file_name
            _config._alpn.c_str(),
			[](picoquic_cnx_t* cnx, uint64_t stream_id, uint8_t* bytes, size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* stream_ctx) -> int { 
                return ((QuicServer*)callback_ctx)->CallbackConnectionInit(cnx, stream_id, bytes, length, fin_or_event, callback_ctx, stream_ctx);
            },
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

        if (!_quic)
        {
            _error = TOFU_MAKE_ERROR("Could not create server context");
            return;
        }

        picoquic_set_cookie_mode(_quic, 2);
        picoquic_set_default_congestion_algorithm(_quic, picoquic_bbr_algorithm);
        picoquic_set_qlog(_quic, _config._config._qlogDirectory.string().c_str());
        picoquic_set_log_level(_quic, _config._config._qlogLevel);
        picoquic_set_key_log_file_from_env(_quic);

		_thread = std::thread{ [this, quic = _quic, port = *_config._port]() {
            fmt::print("[QuicServer] loop start.\n");
            _loopReturnCode = 
#ifdef _WINDOWS
            picoquic_packet_loop_win(
#else
            picoquic_packet_loop(
#endif
                quic, port, 0, 0,
                [](picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx) -> int
                {
                    return ((QuicServer*)callback_ctx)->CallbackPacketLoop(quic, cb_mode, callback_ctx);
				}, this
            );
            fmt::print("[QuicServer] loop exit.\n");
			} 
        };
    }

    void QuicServer::Exit()
    {
        if (!_quic)
            return;

        _end = true;

        if(_thread.joinable())
			_thread.join();

        if (_loopReturnCode != 0 && _loopReturnCode != error_code::Interrupt)
        {
            _error = TOFU_MAKE_ERROR("picoquic_packet_loop_win() did not complete successfully. error_code = {}", _loopReturnCode);
        }

        fmt::print("[QuicServer] Server Exit.\n");

        if (_quic)
        {
            picoquic_free(_quic);
        }

        _quic = nullptr;
    }

    void QuicServer::OnCloseConnection(const std::shared_ptr<QuicConnection>& connection)
    {
        fmt::print("[QuicServer] OnCloseConnection()\n");
        {
            std::lock_guard lock{ _mutexConnections };
            _connections.erase(connection);
        }
    }

    int QuicServer::CallbackConnectionInit(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx)
    {
        fmt::print("[QuicServer] CallbackConnectionInit()\n");

        auto quic = picoquic_get_quic_ctx(cnx);

        auto connection = std::make_shared<QuicConnection>(cnx, this);
        {
            std::lock_guard lock{ _mutexConnections };
			_connections.insert(connection);
        }

        picoquic_set_callback(cnx,
            [](picoquic_cnx_t* cnx, uint64_t stream_id, uint8_t* bytes, size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* stream_ctx) -> int {
                return ((QuicConnection*)callback_ctx)->CallbackConnection(cnx, stream_id, bytes, length, fin_or_event, callback_ctx, stream_ctx);
            },
            connection.get()
		);

        return 0;
    }

    int QuicServer::CallbackPacketLoop(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx)
    {
        // fmt::print("[QuicServer] CallbackPacketLoop(cb_mode = {})\n", cb_mode);
        if (_end)
            return error_code::Interrupt;

        return 0;
    }
}

