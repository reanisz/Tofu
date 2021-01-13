#include <filesystem>
#include <optional>
#include <thread>

#include <tofu/utils/strong_numeric.h>
#include <tofu/utils/observer_ptr.h>

#include <picoquic.h>
#include <picoquic_packet_loop.h>

#include <fmt/core.h>

namespace tofu
{
	struct Error
	{
		std::string _message;
#if _DEBUG
		const char* _file;
		int _line;
#endif

        void Dump() const
        {
            fmt::print("message=<{}>, file={}, line={}\n", _message, _file, _line);
        }
	};

#if _DEBUG
#define TOFU_MAKE_ERROR(...) Error{fmt::format(__VA_ARGS__), __FILE__, __LINE__}
#else
#define TOFU_MAKE_ERROR(...) Error{fmt::format(__VA_ARGS__)}
#endif
}

namespace tofu::net
{
    namespace error_code
    {
        // Tofu固有エラーコードの基点
        inline constexpr int Base = 0x10000000;

        // ユーザー操作によって中断された
        inline constexpr int Interrupt = Base + 1;
    }

    using Port = StrongNumeric<class tag_Port, int>;

    struct QuicConfig
    {
        std::filesystem::path _qlogDirectory;
    };

    struct QuicServerConfig
    {
        QuicConfig _config;

        Port _port;
        std::filesystem::path _certFile;
        std::filesystem::path _secretFile;
        std::string _alpn;
    };

    class QuicServer;

    class QuicServerConnection
    {
    public:
        QuicServerConnection(observer_ptr<QuicServer> server);
        int CallbackConnection(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);

    private:
        observer_ptr<QuicServer> _server;
        std::optional<Error> _error;
    };

    class QuicServer
    {
    public:
        QuicServer(const QuicServerConfig& config);

        void Start();

        int CallbackConnectionInit(picoquic_cnx_t* cnx, std::uint64_t stream_id, std::uint8_t* bytes, std::size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);
        int CallbackPacketLoop(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx);

        bool HasError() const { return _error.has_value(); }
        const Error& GetError() const { return *_error; }

    private:
        QuicServerConfig _config;
        std::optional<Error> _error;

        std::thread _thread;
        std::atomic<bool> _end;
    };
}
