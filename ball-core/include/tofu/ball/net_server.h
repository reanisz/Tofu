#pragma once

#include <memory>

#include <tofu/utils.h>

#include <tofu/net/quic.h>
#include <tofu/net/quic_server.h>

#include <tofu/ball/player.h>
#include <tofu/ball/network.h>

#include <tofu/ball/game.h>

namespace tofu::ball
{
    class ClientConnection
    {
    public:
        enum class State
        {
            WaitConnect,
            WaitJoinRequest,
            Ready,
        };

        ClientConnection(const std::shared_ptr<net::QuicConnection>& quic, PlayerID player_id);

        void Update();
    private:
        void UpdateWaitConnect();
        void UpdateWaitJoinRequest();
    public:

        State GetState() const noexcept
        {
            return _state;
        }

        Error GetError() const noexcept
        {
            return _error;
        }
    private:
        std::shared_ptr<net::QuicConnection> _quic;
        PlayerID _id;
        std::string _name;

        State _state = State::WaitConnect;
        Error _error;

        std::shared_ptr<net::QuicStream> _streamControlSend;
        std::shared_ptr<net::QuicStream> _streamControlRecv;
    };

    class Server
    {
        enum class State
        {
            Init,
            Lobby,
            InGame,
            Exit,
        };
    public:
        Server()
        {
        }

        void Run();
        void Stop();

    private:
        void UpdateAtLobby();

        void InitGame();

    private:
        std::unique_ptr<net::QuicServer> _quic;
        std::atomic<bool> _end = false;

        State _state = State::Init;
        std::atomic<int> _clientNum = 0;

        static constexpr int ClientMax = 2;
        std::array<std::shared_ptr<ClientConnection>, ClientMax> _connections;
        std::mutex _mutexConnection;

        Game _game;
    };

}

