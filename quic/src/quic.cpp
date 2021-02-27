#include "tofu/net/quic.h"
#include "tofu/net/quic_client.h"
#include "tofu/net/quic_server.h"

// #define TOFU_QUIC_LOG(...) TOFU_QUIC_LOG(__VA_ARGS__)
#define TOFU_QUIC_LOG(...)

namespace tofu::net {
    QuicStream::QuicStream(observer_ptr<QuicConnection> connection, StreamId stream_id)
        : _connection(connection)
        , _streamId(stream_id)
        // TODO: buffer_sizeはconfigからとるようにする
        , _recvBuffer(1024 * 1024 * 1024) // 1kB
        , _sendBuffer(1024 * 1024 * 1024) // 1kB
    {
    }

    StreamId QuicStream::GetId() const noexcept
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

    bool QuicStream::IsReceiveFinished()
    {
        std::lock_guard lock{ _recvMutex };
        return _isArrivedFinish && _recvBuffer.Size() == 0;
    }

    void QuicStream::Send(const std::byte* data, std::size_t length)
    {
        // picoquic_add_to_stream_with_ctx(_connection->GetRaw(), *_streamId, reinterpret_cast<const std::uint8_t*>(data), length, false, this);
        std::lock_guard lock{ _sendMutex };
        _sendBuffer.Write(data, length);

        picoquic_mark_active_stream(_connection->GetRaw(), *_streamId, true, this);
    }

    void QuicStream::FinishSend()
    {
        _isSendFinish = true;
        picoquic_mark_active_stream(_connection->GetRaw(), *_streamId, true, this);
    }

    bool QuicStream::IsSendFinished() const
    {
        return _isSendFinish;
    }

    void QuicStream::Close()
    {
        FinishSend();
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
        TOFU_QUIC_LOG("[QuicConnection] constructed as server.\n");

        Init();
    }
    QuicConnection::QuicConnection(picoquic_cnx_t* cnx, observer_ptr<QuicClient> client)
        : _cnx(cnx)
        , _client(client)
        , _config(client->GetConfig()._config)
        , _unreliableRecvBuffer(client->GetConfig()._config._unreliableRecvBufferSize)
    {
        TOFU_QUIC_LOG("[QuicConnection] constructed as client.\n");

        Init();
    }

    QuicConnection::~QuicConnection()
    {
        TOFU_QUIC_LOG("[QuicConnection] destructed.\n");\
    }

    void QuicConnection::Init()
    {
        if (0 < _config._pingInterval.count())
        {
            picoquic_enable_keep_alive(_cnx, _config._pingInterval.count());
        }
    }

    void QuicConnection::CheckSendable()
    {
        return;
        for (auto [id, stream] : _streams)
        {
            if (stream->_sendBuffer.Size())
            {
                picoquic_mark_active_stream(_cnx, *id, true, stream.get());
            }
        }
    }

    void QuicConnection::Close()
    {
        if (_cnx) {
            fmt::print("local_error: {}\nremote_error: {}\n", picoquic_get_local_error(_cnx), picoquic_get_remote_error(_cnx));
            picoquic_set_callback(_cnx, NULL, NULL);
        }

        picoquic_close(_cnx, 0);

        _isDisconnected = true;

        if (_client)
            _client->OnCloseConnection(shared_from_this());
        if (_server)
            _server->OnCloseConnection(shared_from_this());
    }

    std::shared_ptr<QuicStream> QuicConnection::OpenStream(StreamId stream_id, bool is_remote)
    {
        std::lock_guard lock{ _streamMutex };

        if (auto it = _streams.find(stream_id); it != _streams.end())
        {
            return it->second;
        }

        auto ctx = std::make_shared<QuicStream>(this, stream_id);

        if(!is_remote)
            picoquic_reset_stream(_cnx, *stream_id, 0);

        picoquic_set_app_stream_ctx(_cnx, *stream_id, ctx.get());

        // auto ret = picoquic_mark_active_stream(_cnx, stream_id, true, ctx.get());
        // 
        // if (ret != 0)
        // {
        //     return nullptr;
        // }

        _streams.insert({ stream_id, ctx });

        return ctx;
    }

    std::shared_ptr<QuicStream> QuicConnection::GetStream(StreamId stream_id)
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

                if (auto it = _streams.find(stream_id); it != _streams.end())
                {
                    stream_ctx = it->second.get();
                }
                else 
                {
                    auto ctx = std::make_shared<QuicStream>(this, stream_id);
                    stream_ctx = ctx.get();

                    _streams.insert({ stream_id, ctx });
                }

                picoquic_set_app_stream_ctx(cnx, stream_id, stream_ctx);
            }

            stream_ctx->ArriveData(reinterpret_cast<const std::byte*>(bytes), length);
            if (fin_or_event == picoquic_callback_stream_fin)
                stream_ctx->ArriveFinish();
            break;
        case picoquic_callback_stream_reset:
        {
            assert(false);
        }
            break;
        case picoquic_callback_prepare_to_send:
            /* Active sending API */
            // TOFU_QUIC_LOG("[QuicConnection] prepare_to_send.\n");
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
            TOFU_QUIC_LOG("[QuicConnection] Connection to the server completed, almost ready.\n");
            break;
        case picoquic_callback_ready:
        {
            TOFU_QUIC_LOG("[QuicConnection] Connection to the server confirmed.\n");
            _isReady = true;
            break;
        }
        default:
            TOFU_QUIC_LOG("[QuicConnection] unexpected event : {}\n", fin_or_event);
            break;
        }
        return 0;

    }
}
