#include "tofu/net/quic.h"
#include "tofu/net/quic_client.h"
#include "tofu/net/quic_server.h"

namespace tofu::net {
    QuicStream::QuicStream(observer_ptr<QuicConnection> connection, std::uint64_t stream_id)
        : _connection(connection)
        , _streamId(stream_id)
        // TODO: buffer_size‚Íconfig‚©‚ç‚Æ‚é‚æ‚¤‚É‚·‚é
        , _recvBuffer(2 * 1024 * 1024) // 2kB
        , _sendBuffer(2 * 1024 * 1024) // 2kB
    {
    }

    std::int64_t QuicStream::GetId() const noexcept
    {
        return _streamId;
    }

    std::size_t QuicStream::ReceivedSize() noexcept
    {
        std::lock_guard lock{ _recvMutex };
        return _recvBuffer.Size();
    }

    void QuicStream::Peek(std::byte* data, std::size_t length) 
    {
        std::lock_guard lock{ _recvMutex };
        _recvBuffer.Peek(data, length);
    }

    void QuicStream::Read(std::byte* data, std::size_t length)
    {
        std::lock_guard lock{ _recvMutex };
        _recvBuffer.Read(data, length);
    }

    void QuicStream::Seek(std::size_t length)
    {
        std::lock_guard lock{ _recvMutex };
        _recvBuffer.Seek(length);
    }

    void QuicStream::Send(const std::byte* data, std::size_t length)
    {
        std::lock_guard lock{ _sendMutex };
        _sendBuffer.Write(data, length);

        picoquic_mark_active_stream(_connection->GetRaw(), _streamId, true, this);
    }

    void QuicStream::FinishSend()
    {
        _isSendFinish = true;
    }

    void QuicStream::ArriveData(const std::byte* data, std::size_t length)
    {
        _recvBuffer.Write(data, length);
    }

    void QuicStream::ArriveFinish()
    {
        _isArrivedFinish = true;
    }

    QuicReturnCode QuicStream::OnPrepareToSend(void* context, int length)
    {
        std::lock_guard lock{ _sendMutex };

        auto send_length = std::min<std::size_t>(length, _sendBuffer.Size());
        if (!send_length)
            return 0;

        bool is_fin = _isSendFinish;

        auto buffer = picoquic_provide_stream_data_buffer(context, send_length, is_fin, _sendBuffer.Remain() == length);
        if (!buffer)
            return -1;

        _sendBuffer.Read(reinterpret_cast<std::byte*>(buffer), send_length);

        return 0;
    }

	QuicConnection::QuicConnection(picoquic_cnx_t* cnx, observer_ptr<QuicServer> server)
		: _cnx(cnx)
		, _server(server)
        , _config(server->GetConfig()._config)
        , _unreliableRecvBuffer(server->GetConfig()._config._unreliableRecvBufferSize)
	{
        fmt::print("[QuicConnection] constructed as server.\n");

        Init();
	}
	QuicConnection::QuicConnection(picoquic_cnx_t* cnx, observer_ptr<QuicClient> client)
		: _cnx(cnx)
		, _client(client)
        , _config(client->GetConfig()._config)
        , _unreliableRecvBuffer(client->GetConfig()._config._unreliableRecvBufferSize)
	{
        fmt::print("[QuicConnection] constructed as client.\n");

        Init();
	}

	QuicConnection::~QuicConnection()
	{
        fmt::print("[QuicConnection] destructed.\n");
	}

    void QuicConnection::Init()
    {
        if (0 < _config._pingInterval.count())
        {
            picoquic_enable_keep_alive(_cnx, _config._pingInterval.count());
        }
    }

    void QuicConnection::Close()
    {
        if (_cnx) {
            picoquic_set_callback(_cnx, NULL, NULL);
        }

        picoquic_close(_cnx, 0);

        if (_client)
            _client->OnCloseConnection(shared_from_this());
        if (_server)
            _server->OnCloseConnection(shared_from_this());
	}

    std::shared_ptr<QuicStream> QuicConnection::OpenStream(std::uint64_t stream_id)
    {
        std::lock_guard lock{ _streamMutex };
        auto ctx = std::make_shared<QuicStream>(this, stream_id);

        auto ret = picoquic_mark_active_stream(_cnx, stream_id, true, ctx.get());

        if (ret != 0)
        {
            return nullptr;
        }

        _streams.insert({ stream_id, ctx });

        return ctx;
    }

    std::shared_ptr<QuicStream> QuicConnection::GetStream(std::uint64_t stream_id)
    {
        std::lock_guard lock{ _streamMutex };
        if (auto it = _streams.find(stream_id); it != _streams.end())
        {
            return it->second;
        }
        else
        {
            return nullptr;
        }
    }

    void QuicConnection::SendUnreliable(observer_ptr<const std::byte> data, std::size_t size)
    {
        picoquic_queue_datagram_frame(_cnx, size, reinterpret_cast<const std::uint8_t*>(data.get()));
    }

    std::size_t QuicConnection::ReceivedUnreliableCount()
    {
		std::lock_guard lock{ _unreliableRecvMutex };
        return _unreliableRecvBuffer.Count();
    }

    std::size_t QuicConnection::GetUnreliableTopSize()
    {
		std::lock_guard lock{ _unreliableRecvMutex };
        return _unreliableRecvBuffer.Peek()._length;
    }

    std::size_t QuicConnection::ReadUnreliable(std::byte* dest, std::size_t dest_size)
    {
		std::lock_guard lock{ _unreliableRecvMutex };
        if (!_unreliableRecvBuffer.Count())
            return 0;

        std::size_t res = 0;
        {
            auto data = _unreliableRecvBuffer.Pop();
            assert(data._length <= dest_size);
            data.CopyTo(dest);
            res = data._length;
        }
        return res;
    }

	int QuicConnection::CallbackConnection(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx)
	{
        assert(_cnx == cnx || cnx == nullptr);

        QuicStream* stream_ctx = (QuicStream*)v_stream_ctx;

        switch (fin_or_event)
        {
		case picoquic_callback_stream_data:
        case picoquic_callback_stream_fin:
            /* Data arrival on stream #x, maybe with fin mark */
            if (!stream_ctx) {
                std::lock_guard lock{ _streamMutex };

                auto ctx = std::make_shared<QuicStream>(this, stream_id);
                stream_ctx = ctx.get();

                _streams.insert({ stream_id, ctx });

                picoquic_set_app_stream_ctx(cnx, stream_id, stream_ctx);
            }

            stream_ctx->ArriveData(reinterpret_cast<const std::byte*>(bytes), length);
            if (fin_or_event == picoquic_callback_stream_fin)
                stream_ctx->ArriveFinish();
            break;
        case picoquic_callback_prepare_to_send:
            /* Active sending API */
            fmt::print("[QuicConnection] prepare_to_send.\n");
            assert(stream_ctx);

            return *stream_ctx->OnPrepareToSend(bytes, length);
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
            std::lock_guard lock{ _unreliableRecvMutex };
            _unreliableRecvBuffer.Write(reinterpret_cast<std::byte*>(bytes), length);
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
