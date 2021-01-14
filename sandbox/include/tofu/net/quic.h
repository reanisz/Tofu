#pragma once

#include <filesystem>
#include <optional>
#include <thread>
#include <set>

#include <tofu/utils/strong_numeric.h>
#include <tofu/utils/observer_ptr.h>
#include <tofu/utils/error.h>

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

    struct QuicConfig
    {
        std::filesystem::path _qlogDirectory;
        int _qlogLevel = 1;
    };

    class QuicConnection;
    class QuicServer;
    class QuicClient;

    class QuicStream
        : public std::enable_shared_from_this<QuicStream>
    {
    public:
        QuicStream(observer_ptr<QuicConnection> connection, std::uint64_t stream_id);

    protected:
        friend class QuicConnection;
        void ArriveData(const std::byte* data, std::size_t length);
        void ArriveFinish();

        QuicReturnCode OnPrepareToSend(void* context, int length);

    private:
        observer_ptr<QuicConnection> _connection;
        std::uint64_t _streamId;

        std::atomic<bool> _isArrivedFinish = false;
    };

    class QuicConnection
        : public std::enable_shared_from_this<QuicConnection>
    {
    public:
        QuicConnection(picoquic_cnx_t* cnx, observer_ptr<QuicServer> server);
        QuicConnection(picoquic_cnx_t* cnx, observer_ptr<QuicClient> server);

        ~QuicConnection();

        void Close();

        void SendUnreliable(observer_ptr<const std::byte> data, std::size_t size);

        int CallbackConnection(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);

    private:
        picoquic_cnx_t* _cnx;
        // とりあえず別々に持ったけどインターフェース切るなりしたほうがいいか後で考える
        observer_ptr<QuicServer> _server = nullptr;
        observer_ptr<QuicClient> _client = nullptr;

        std::set<std::shared_ptr<QuicStream>> _streams;
    };
}

