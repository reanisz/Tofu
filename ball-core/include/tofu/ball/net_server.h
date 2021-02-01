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
    class Server;

    class ClientConnection
    {
    public:
        enum class State
        {
            WaitConnect,
            WaitJoinRequest,
            Ready,
            Ingame,
        };

        ClientConnection(observer_ptr<Server> server, const std::shared_ptr<net::QuicConnection>& quic, PlayerID player_id);

        void Update();

        void StartGame();
    private:
        void UpdateWaitConnect();
        void UpdateWaitJoinRequest();
        void UpdateIngame();
    public:

        State GetState() const noexcept
        {
            return _state;
        }

        Error GetError() const noexcept
        {
            return _error;
        }

        PlayerID GetID() const noexcept
        {
            return _id;
        }

        const std::string& GetName() const noexcept
        {
            return _name;
        }

    private:
        std::shared_ptr<net::QuicConnection> _quic;
        observer_ptr<Server> _server;

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

        virtual ~Server()
        {
        }

        void Run();
        void Stop();

        const std::array<std::shared_ptr<ClientConnection>, MaxPlayerNum> GetConnections() const noexcept
        {
            return _connections;
        }

    private:
        void UpdateAtLobby();
        void UpdateAtIngame();

    protected:
        virtual void InitGame();

    private:
        std::unique_ptr<net::QuicServer> _quic;
        std::atomic<bool> _end = false;

        State _state = State::Init;
        std::atomic<int> _clientNum = 0;

        std::array<std::shared_ptr<ClientConnection>, MaxPlayerNum> _connections;
        std::mutex _mutexConnection;

        Game _game;
    };

}

