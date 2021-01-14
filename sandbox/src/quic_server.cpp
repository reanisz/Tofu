#include <fmt/core.h>

#include <picoquic.h>
#include <picoquic_utils.h>
#include <picoquic_packet_loop.h>
#include <picosocks.h>
#include <autoqlog.h>

#include "tofu/net/quic_server.h"

namespace tofu::net
{
    QuicServerConnection::QuicServerConnection(observer_ptr<QuicServer> server)
        : _server(server)
    {
        fmt::print("[QuicServerConnection] new\n");
    }

    int QuicServerConnection::CallbackConnection(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx)
    {
        switch (fin_or_event)
        {
		// case picoquic_callback_stream_data:
        // case picoquic_callback_stream_fin:
        //     /* Data arrival on stream #x, maybe with fin mark */
        //     if (stream_ctx == NULL) {
        //         /* Create and initialize stream context */
        //         stream_ctx = sample_server_create_stream_context(server_ctx, stream_id);
        //     }

        //     if (stream_ctx == NULL) {
        //         /* Internal error */
        //         (void) picoquic_reset_stream(cnx, stream_id, PICOQUIC_SAMPLE_INTERNAL_ERROR);
        //         return(-1);
        //     }
        //     else if (stream_ctx->is_name_read) {
        //         /* Write after fin? */
        //         return(-1);
        //     }
        //     else {
        //         /* Accumulate data */
        //         size_t available = sizeof(stream_ctx->file_name) - stream_ctx->name_length - 1;

        //         if (length > available) {
        //             /* Name too long: reset stream! */
        //             sample_server_delete_stream_context(server_ctx, stream_ctx);
        //             (void) picoquic_reset_stream(cnx, stream_id, PICOQUIC_SAMPLE_NAME_TOO_LONG_ERROR);
        //         }
        //         else {
        //             if (length > 0) {
        //                 memcpy(stream_ctx->file_name + stream_ctx->name_length, bytes, length);
        //                 stream_ctx->name_length += length;
        //             }
        //             if (fin_or_event == picoquic_callback_stream_fin) {
        //                 int stream_ret;

        //                 /* If fin, mark read, check the file, open it. Or reset if there is no such file */
        //                 stream_ctx->file_name[stream_ctx->name_length + 1] = 0;
        //                 stream_ctx->is_name_read = 1;
        //                 stream_ret = sample_server_open_stream(server_ctx, stream_ctx);

        //                 if (stream_ret == 0) {
        //                     /* If data needs to be sent, set the context as active */
        //                     ret = picoquic_mark_active_stream(cnx, stream_id, 1, stream_ctx);
        //                 }
        //                 else {
        //                     /* If the file could not be read, reset the stream */
        //                     sample_server_delete_stream_context(server_ctx, stream_ctx);
        //                     (void) picoquic_reset_stream(cnx, stream_id, stream_ret);
        //                 }
        //             }
        //         }
        //     }
        //     break;
        // case picoquic_callback_prepare_to_send:
        //     /* Active sending API */
        //     if (stream_ctx == NULL) {
        //         /* This should never happen */
        //     }
        //     else if (stream_ctx->F == NULL) {
        //         /* Error, asking for data after end of file */
        //     }
        //     else {
        //         /* Implement the zero copy callback */
        //         size_t available = stream_ctx->file_length - stream_ctx->file_sent;
        //         int is_fin = 1;
        //         uint8_t* buffer;

        //         if (available > length) {
        //             available = length;
        //             is_fin = 0;
        //         }
        //         
        //         buffer = picoquic_provide_stream_data_buffer(bytes, available, is_fin, !is_fin);
        //         if (buffer != NULL) {
        //             size_t nb_read = fread(buffer, 1, available, stream_ctx->F);

        //             if (nb_read != available) {
        //                 /* Error while reading the file */
        //                 sample_server_delete_stream_context(server_ctx, stream_ctx);
        //                 (void)picoquic_reset_stream(cnx, stream_id, PICOQUIC_SAMPLE_FILE_READ_ERROR);
        //             }
        //             else {
        //                 stream_ctx->file_sent += available;
        //             }
        //         }
        //         else {
        //         /* Should never happen according to callback spec. */
        //             ret = -1;
        //         }
        //     }
        //     break;
        // case picoquic_callback_stream_reset: /* Client reset stream #x */
        // case picoquic_callback_stop_sending: /* Client asks server to reset stream #x */
        //     if (stream_ctx != NULL) {
        //         /* Mark stream as abandoned, close the file, etc. */
        //         sample_server_delete_stream_context(server_ctx, stream_ctx);
        //         picoquic_reset_stream(cnx, stream_id, PICOQUIC_SAMPLE_FILE_CANCEL_ERROR);
        //     }
        //     break;
        case picoquic_callback_datagram:
        {
            std::string_view view{ reinterpret_cast<char*>(bytes), length };
            fmt::print("[QuicServerConnection] received datagram: <{}>\n", view);

        }
            break;
        case picoquic_callback_stateless_reset: /* Received an error message */
        case picoquic_callback_close: /* Received connection close */
        case picoquic_callback_application_close: /* Received application close */
            /* Delete the server application context */
            fmt::print("[QuicServerConnection] delete this.\n");
            picoquic_set_callback(cnx, NULL, NULL);
            delete this;
            return 0;
        case picoquic_callback_version_negotiation:
            /* The server should never receive a version negotiation response */
            break;
        case picoquic_callback_stream_gap:
            /* This callback is never used. */
            break;
        case picoquic_callback_almost_ready:
        case picoquic_callback_ready:
            /* Check that the transport parameters are what the sample expects */
            break;
        default:
            /* unexpected */
            break;
        }
        return 0;
    }

    QuicServer::QuicServer(const QuicServerConfig& config)
        : _config(config)
        , _end(false)
    {
    }

    void QuicServer::Start()
    {
        fmt::print("[QuicServer] Starting server on port {}\n", *_config._port);
        auto current_time = picoquic_current_time();

        picoquic_quic_t* quic = picoquic_create(
            8, // nb_connections
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

        if (!quic)
        {
            _error = TOFU_MAKE_ERROR("Could not create server context");
            return;
        }

        picoquic_set_cookie_mode(quic, 2);
        picoquic_set_default_congestion_algorithm(quic, picoquic_bbr_algorithm);
        picoquic_set_qlog(quic, _config._config._qlogDirectory.string().c_str());
        picoquic_set_log_level(quic, _config._config._qlogLevel);
        picoquic_set_key_log_file_from_env(quic);

        std::atomic<int> loop_ret = 0;

		_thread = std::thread{ [this, quic, port = *_config._port, &loop_ret]() {
            fmt::print("[QuicServer] loop start.\n");
            loop_ret = 
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

        fmt::print("[QuicServer] Press key to exit\n");
        getchar();

        _end = true;

        if(_thread.joinable())
			_thread.join();

        if (loop_ret != 0 && loop_ret != error_code::Interrupt)
        {
            _error = TOFU_MAKE_ERROR("picoquic_packet_loop_win() did not complete successfully. error_code = {}", loop_ret);
        }

        fmt::print("[QuicServer] Server Exit.\n");

        if (quic)
        {
            picoquic_free(quic);
        }
    }

    int QuicServer::CallbackConnectionInit(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx)
    {
        fmt::print("[QuicServer] CallbackConnectionInit()\n");

        auto quic = picoquic_get_quic_ctx(cnx);

        picoquic_set_callback(cnx,
            [](picoquic_cnx_t* cnx, uint64_t stream_id, uint8_t* bytes, size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* stream_ctx) -> int {
                return ((QuicServerConnection*)callback_ctx)->CallbackConnection(cnx, stream_id, bytes, length, fin_or_event, callback_ctx, stream_ctx);
            },
            new QuicServerConnection(this)
		);

        return 0;
    }

    int QuicServer::CallbackPacketLoop(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx)
    {
        fmt::print("[QuicServer] CallbackPacketLoop(cb_mode = {})\n", cb_mode);
        if (_end)
            return error_code::Interrupt;

        return 0;
    }
}

