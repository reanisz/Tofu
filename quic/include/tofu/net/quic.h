#pragma once

#include <filesystem>
#include <optional>
#include <thread>
#include <set>
#include <unordered_map>
#include <mutex>
#include <chrono>

#include <tofu/utils/strong_numeric.h>
#include <tofu/utils/observer_ptr.h>
#include <tofu/utils/error.h>
#include <tofu/utils/circular_queue_allocator.h>

#include <picoquic.h>
#include <picoquic_packet_loop.h>

namespace tofu::net
{
    namespace error_code
    {
        // Tofu固有エラーコードの基点
        inline constexpr int Base = 0x10000000;

        // ユーザー操作によって中断された
        inline constexpr int Interrupt = Base + 1;

        // コンテキストが見つからなかった
        inline constexpr int ContextNotFound = Base + 2;
    }

    using Port = StrongNumeric<class tag_Port, int>;
    using QuicReturnCode = StrongNumeric<class tag_QuicReturnCode, int>;
    using StreamId = StrongNumeric<class tag_StreamId, std::uint64_t>;

    struct QuicConfig
    {
        std::filesystem::path _qlogDirectory;
        int _qlogLevel = 1;

        std::size_t _unreliableRecvBufferSize = 2 * 1024 * 1024;

        std::chrono::microseconds _pingInterval = std::chrono::microseconds{ 100 * 1000 };
    };

    class QuicConnection;
    class QuicServer;
    class QuicClient;

    class QuicStream
        : public std::enable_shared_from_this<QuicStream>
    {
    public:
        QuicStream(observer_ptr<QuicConnection> connection, StreamId stream_id);

        StreamId GetId() const noexcept;

        std::size_t ReceivedSize() noexcept;
        void Peek(std::byte* data, std::size_t length);
        void Read(std::byte* data, std::size_t length);
        void Seek(std::size_t length);
        bool IsReceiveFinished();

        void Send(const std::byte* data, std::size_t length);
        void FinishSend();
        bool IsSendFinished() const;

        void Close();

    protected:
        friend class QuicConnection;
        void ArriveData(const std::byte* data, std::size_t length);
        void ArriveFinish();

        QuicReturnCode OnPrepareToSend(void* context, int length);

    private:
        observer_ptr<QuicConnection> _connection;
        StreamId _streamId;

        std::mutex _recvMutex;
        std::atomic<bool> _isArrivedFinish = false;
        CircularContinuousBuffer _recvBuffer;

        std::mutex _sendMutex;
        std::atomic<bool> _isSendFinish = false;
        CircularContinuousBuffer _sendBuffer;
    };

    class QuicConnection
        : public std::enable_shared_from_this<QuicConnection>
    {
    public:
        QuicConnection(picoquic_cnx_t* cnx, observer_ptr<QuicServer> server);
        QuicConnection(picoquic_cnx_t* cnx, observer_ptr<QuicClient> client);

        ~QuicConnection();

    private:
        void Init();
    public:

        void CheckSendable();
        void Close();

        picoquic_cnx_t* GetRaw() const
        {
            return _cnx;
        }

        bool IsConnected() const
        {
            return _isReady && !_isDisconnected;
        }

        bool IsDisconnected() const
        {
            return _isDisconnected;
        }

        // === Stream
        std::shared_ptr<QuicStream> OpenStream(StreamId stream_id, bool is_remote);
        std::shared_ptr<QuicStream> GetStream(StreamId stream_id);

        // === DATAGRAM
        void SendUnreliable(observer_ptr<const std::byte> data, std::size_t size);
        std::size_t ReceivedUnreliableCount();
        std::size_t GetUnreliableTopSize();
        std::size_t ReadUnreliable(std::byte* dest, std::size_t dest_size);

        int CallbackConnection(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);

    private:
        picoquic_cnx_t* _cnx;
        std::atomic<bool> _isReady = false;
        std::atomic<bool> _isDisconnected = false;

        // とりあえず別々に持ったけどインターフェース切るなりしたほうがいいか後で考える
        observer_ptr<QuicServer> _server = nullptr;
        observer_ptr<QuicClient> _client = nullptr;

        QuicConfig _config;

        std::mutex _unreliableRecvMutex;
        CircularQueueBuffer _unreliableRecvBuffer;

        std::mutex _streamMutex;
        std::unordered_map<StreamId, std::shared_ptr<QuicStream>> _streams;
    };
}

