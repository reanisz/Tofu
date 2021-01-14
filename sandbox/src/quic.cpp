#include "tofu/net/quic.h"
#include "tofu/net/quic_client.h"
#include "tofu/net/quic_server.h"

namespace tofu::net {
    QuicStream::QuicStream(observer_ptr<QuicConnection> connection, std::uint64_t stream_id)
        : _connection(connection)
        , _streamId(stream_id)
    {
    }

    void QuicStream::ArriveData(const std::byte* data, std::size_t length)
    {
    }

    void QuicStream::ArriveFinish()
    {
    }

    QuicReturnCode QuicStream::OnPrepareToSend(void* context, int length)
    {
        return QuicReturnCode();
    }

	QuicConnection::QuicConnection(picoquic_cnx_t* cnx, observer_ptr<QuicServer> server)
		: _cnx(cnx)
		, _server(server)
	{
        fmt::print("[QuicConnection] constructed as server.\n");
	}
	QuicConnection::QuicConnection(picoquic_cnx_t* cnx, observer_ptr<QuicClient> client)
		: _cnx(cnx)
		, _client(client)
	{
        fmt::print("[QuicConnection] constructed as client.\n");
	}

	QuicConnection::~QuicConnection()
	{
        fmt::print("[QuicConnection] destructed.\n");
	}

    void QuicConnection::Close()
    {
        if (_cnx) {
            picoquic_set_callback(_cnx, NULL, NULL);
        }

        if (_client)
            _client->OnCloseConnection(shared_from_this());
        if (_server)
            _server->OnCloseConnection(shared_from_this());
	}

    void QuicConnection::SendUnreliable(observer_ptr<const std::byte> data, std::size_t size)
    {
        picoquic_queue_datagram_frame(_cnx, size, reinterpret_cast<const std::uint8_t*>(data.get()));
    }

	int QuicConnection::CallbackConnection(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx)
	{
        assert(_cnx == cnx);

        QuicStream* stream_ctx = (QuicStream*)v_stream_ctx;

        switch (fin_or_event)
        {
		case picoquic_callback_stream_data:
        case picoquic_callback_stream_fin:
            /* Data arrival on stream #x, maybe with fin mark */
            if (!stream_ctx) {
                auto ctx = std::make_shared<QuicStream>(cnx, stream_id);
                stream_ctx = ctx.get();

                _streams.insert(ctx);
            }

            stream_ctx->ArriveData(reinterpret_cast<const std::byte*>(bytes), length);
            if (fin_or_event == picoquic_callback_stream_fin)
                stream_ctx->ArriveFinish();
            break;
        case picoquic_callback_prepare_to_send:
            /* Active sending API */
            assert(stream_ctx);

            stream_ctx->OnPrepareToSend(bytes, length);
            break;
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

            picoquic_close(cnx, 0);
        }
            break;
        case picoquic_callback_stateless_reset: /* Received an error message */
        case picoquic_callback_close: /* Received connection close */
        case picoquic_callback_application_close: /* Received application close */
            /* Delete the server application context */
            Close();
            return 0;
        case picoquic_callback_version_negotiation:
            /* The server should never receive a version negotiation response */
            break;
        case picoquic_callback_stream_gap:
            /* This callback is never used. */
            break;
		case picoquic_callback_almost_ready:
			fmt::print("[QuicConnection] Connection to the server completed, almost ready.\n");
			break;
		case picoquic_callback_ready:
		{
			/* TODO: Check that the transport parameters are what the sample expects */
			fmt::print("[QuicConnection] Connection to the server confirmed.\n");

            if (_client)
            {
                const char buf[] = "abcdefghijkl";
                SendUnreliable(reinterpret_cast<const std::byte*>(buf), sizeof(buf));
            }
			break;
		}
		default:
			fmt::print("[QuicConnection] unexpected event : {}\n", fin_or_event);
			/* unexpected -- just ignore. */
			break;
        }
        return 0;

	}
}
