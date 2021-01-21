#include <fmt/core.h>

#include <picoquic.h>
#include <picoquic_utils.h>
#include <picoquic_packet_loop.h>
#include <picosocks.h>
#include <autoqlog.h>

#include "tofu/net/quic_client.h"

namespace tofu::net
{
	QuicClient::QuicClient(const QuicClientConfig& config)
		: _config(config)
	{
	}

	QuicClient::~QuicClient()
	{
		Exit();
	}

	void QuicClient::Start()
	{
		auto current_time = picoquic_current_time();
		sockaddr_storage address;
		int is_name;

		std::string sni = "reanisz.info";

		if (int ret = picoquic_get_server_address(_config._serverName.c_str(), *_config._port, &address, &is_name); ret != 0)
		{
			_error = TOFU_MAKE_ERROR("Cannot get the IP address for {}:{}", _config._serverName, *_config._port);
			return;
		}

		if (is_name)
			sni = _config._serverName;

		_quic = picoquic_create(1, nullptr, nullptr, nullptr,
			_config._alpn.c_str(), nullptr, nullptr, nullptr, nullptr, nullptr,
			current_time, nullptr,
			nullptr, // ticket_file_name
			nullptr, 0);

		if (!_quic)
		{
			_error = TOFU_MAKE_ERROR("Could not create quic context");
			return;
		}

		// picoquic_load_retry_tokens

		picoquic_set_default_congestion_algorithm(_quic, picoquic_bbr_algorithm);

		picoquic_set_key_log_file_from_env(_quic);
		if (!_config._config._qlogDirectory.empty()) 
		{
			picoquic_set_qlog(_quic, _config._config._qlogDirectory.string().c_str());
			picoquic_set_log_level(_quic, _config._config._qlogLevel);
		}

		// ==================================
		fmt::print("[QuicClient] Starting connection to {}:{}\n", _config._serverName, *_config._port);

		auto cnx = picoquic_create_cnx(_quic, picoquic_null_connection_id, picoquic_null_connection_id, (sockaddr*)&address, current_time, 0, sni.c_str(), _config._alpn.c_str(), 1);

		if (!cnx)
		{
			_error = TOFU_MAKE_ERROR("Could not create connection context");
			return;
		}

		_connection = std::make_shared<QuicConnection>(cnx, this);

		picoquic_set_callback(cnx,
            [](picoquic_cnx_t* cnx, uint64_t stream_id, uint8_t* bytes, size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* stream_ctx) -> int {
                return ((QuicConnection*)callback_ctx)->CallbackConnection(cnx, stream_id, bytes, length, fin_or_event, callback_ctx, stream_ctx);
            },
			_connection.get()
			);

		if (int ret = picoquic_start_client_cnx(cnx); ret < 0)
		{
			_error = TOFU_MAKE_ERROR("Could not activate connection.");
			return;
		}

		// TODO: create_stream

		auto icid = picoquic_get_initial_cnxid(cnx);
		fmt::print("[QuicClient] Initial connection ID: ");
		for (int i = 0; i < icid.id_len; i++) 
		{
			fmt::print("{:0x}", icid.id[i]);
		}
		fmt::print("\n");

		std::atomic<int> _loopReturnCode = 0;

		_thread = std::thread{ [this, quic = _quic, address, &_loopReturnCode]() {
			fmt::print("[QuicClient] loop start.\n");
			_loopReturnCode =
	#ifdef _WINDOWS
			picoquic_packet_loop_win(
	#else
			picoquic_packet_loop(
	#endif
				quic, 0, address.ss_family, 0,
				[](picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx) -> int
				{
					return ((QuicClient*)callback_ctx)->CallbackPacketLoop(quic, cb_mode, callback_ctx);
				}, this
				);
			fmt::print("[QuicClient] loop exit.\n");
		} };

		return;
	}

	void QuicClient::Exit()
	{
		if (_connection)
			_connection->Close();

        _end = true;

        if(_thread.joinable())
			_thread.join();

        if (_loopReturnCode != 0 && _loopReturnCode != error_code::Interrupt)
        {
            _error = TOFU_MAKE_ERROR("picoquic_packet_loop_win() did not complete successfully. error_code = {}", _loopReturnCode);
        }

		if (_quic)
			picoquic_free(_quic);

		_quic = nullptr;
		_connection = nullptr;
	}

	std::shared_ptr<QuicConnection> tofu::net::QuicClient::GetConnection()
	{
		return _connection;
	}

	void QuicClient::OnCloseConnection(const std::shared_ptr<QuicConnection>& connection)
	{
        fmt::print("[QuicClient] OnCloseConnection()\n");
        _end = true;
	}

    int QuicClient::CallbackPacketLoop(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx)
    {
        // fmt::print("[QuicClient] CallbackPacketLoop(cb_mode = {})\n", cb_mode);
        if (_end)
            return PICOQUIC_NO_ERROR_TERMINATE_PACKET_LOOP;

        return 0;
    }
}

