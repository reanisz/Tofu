#include <random>

#include "tofu/ball/net_client.h"

namespace tofu::ball
{
    ServerConnection::ServerConnection(const std::shared_ptr<net::QuicConnection>& quic)
        : _quic(quic)
    {
        _streamControlSend = quic->OpenStream(ClientControlStreamId);
        _streamControlRecv = quic->OpenStream(ServerControlStreamId);
    }

    void ServerConnection::Update()
    {
        switch (_state)
        {
        case State::WaitConnect:
            UpdateWaitConnect();
            return;
        case State::WaitJoinApproval:
            UpdateWaitJoinApproval();
            return;
        case State::Ready:
            UpdateReady();
            return;
        }
    }

    void ServerConnection::UpdateWaitConnect()
    {
        if (!_quic->IsConnected())
            return;

        message_client_control::RequestJoin msg;
        std::string user_name = "User";
        user_name += std::to_string(std::random_device{}() % 10000);
        strncpy(msg._userName, user_name.c_str(), sizeof(msg._userName));

        _streamControlSend->Send(reinterpret_cast<const std::byte*>(&msg), sizeof(msg));

        _state = State::WaitJoinApproval;
    }

    void ServerConnection::UpdateWaitJoinApproval()
    {
        auto [message, error] = ReadMessage<message_server_control::ApproveJoin>(_streamControlRecv);

        if (error)
        {
            _error = error;
            return;
        }

        if (!message)
            return;

        _id = message->_playerId;
        _state = State::Ready;
    }

    void ServerConnection::UpdateReady()
    {
        auto [message, error] = ReadMessage<message_server_control::StartGame>(_streamControlRecv);

        if (error)
        {
            _error = error;
            return;
        }

        if (!message)
            return;

        _state = State::InGame;
    }

    void Client::Run()
    {
        net::QuicClientConfig config =
        {
            ._config = {
                ._qlogDirectory = "./qlog/"
            },
            ._serverName = "127.0.0.1",
            ._port = 8000,
            ._alpn = Alpn,
        };

        _quic = std::make_unique<net::QuicClient>(config);
        _quic->Start();

        _state = State::Lobby;

        // 接続が切れたらすぐ終了する
        _quic->SetCallbackOnClose(
            [this](const std::shared_ptr<net::QuicConnection>& connection)
            {
                Stop();
            }
        );

        _connection = std::make_shared<ServerConnection>(_quic->GetConnection());
    }
    void Client::Stop()
    {
        _end = true;
        _state = State::Exit;
    }
    void Client::Exit()
    {
        if (_quic)
        {
            _connection = nullptr;
            _quic->Exit();
            _quic = nullptr;
        }
    }
    void Client::UpdateAtLobby()
    {
        if (!_end && _state == State::Lobby)
        {
            _connection->Update();
        }
        if (_connection->GetState() == ServerConnection::State::InGame) 
        {
            InitGame();
            _game.start();
            _state = State::InGame;
        }
    }

    void Client::UpdateIngame()
    {
        if (!_end && _state == State::InGame)
        {
            _connection->Update();
            UpdateGame();
        }
    }

    void Client::InitGame()
    {
        _game.initBaseSystems();
        _game.initEnitites();
    }
    void Client::UpdateGame()
    {
        _game.update();
    }
}
