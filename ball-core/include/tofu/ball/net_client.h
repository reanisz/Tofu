#pragma once

#include <memory>

#include <tofu/utils.h>

#include <tofu/net/quic.h>
#include <tofu/net/quic_client.h>

#include <tofu/ball/player.h>
#include <tofu/ball/network.h>

#include <tofu/ball/game.h>

namespace tofu::ball
{
	class ServerConnection
	{
    public:
        enum class State
        {
            WaitConnect,
            WaitJoinApproval,
            Ready,
        };

        ServerConnection(const std::shared_ptr<net::QuicConnection>& quic);

        void Update();
    private:
        void UpdateWaitConnect();
        void UpdateWaitJoinApproval();
    public:

        State GetState() const noexcept
        {
            return _state;
        }

        Error GetError() const noexcept
        {
            return _error;
        }

        PlayerID GetPlayerID() const noexcept
        {
            return _id;
        }

    private:
        std::shared_ptr<net::QuicConnection> _quic;
        PlayerID _id = -1;
        std::string _name;

        State _state = State::WaitConnect;
        Error _error;

        std::shared_ptr<net::QuicStream> _streamControlSend;
        std::shared_ptr<net::QuicStream> _streamControlRecv;
	};

	class Client
	{
	public:
        enum class State
        {
            Init,
            Lobby,
            InGame,
            Exit,
        };

		struct Config
		{
		};

		Client(const Config& config)
			: _config(config)
		{
		}
        
        void Run();
        void Stop();

    private:
        void UpdateAtLobby();

	private:
		Config _config;

		std::unique_ptr<net::QuicClient> _quic;
		std::atomic<bool> _end = false;

        State _state = State::Init;
        std::shared_ptr<ServerConnection> _connection;

		Game _game;
	};
}
