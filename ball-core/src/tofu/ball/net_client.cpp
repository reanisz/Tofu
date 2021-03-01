#include <random>

#include "tofu/ball/net_client.h"
#include "tofu/ball/sync.h"

namespace tofu::ball
{
    ServerConnection::ServerConnection(observer_ptr<Client> client, const std::shared_ptr<net::QuicConnection>& quic)
        : _client(client)
        , _quic(quic)
    {
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
        case State::InGame:
            UpdateIngame();
            return;
        }
    }

    void ServerConnection::UpdateWaitConnect()
    {
        if (!_quic->IsConnected())
            return;

        _streamControlSend = _quic->OpenStream(ClientControlStreamId, false);
        _streamControlRecv = _quic->OpenStream(ServerControlStreamId, true);

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

    void ServerConnection::UpdateIngame()
    {
        auto header = PeekHeader(_streamControlRecv);
        if (!header)
            return;
        switch (header->_messageType)
        {
        case message_server_control::SyncPlayerAction::message_type:
        {
            auto [message, error] = ReadMessage<message_server_control::SyncPlayerAction>(_streamControlRecv);
            if (message)
            {
                _client->OnReceiveSyncObject(*message);
            }
        }
            break;
        }
    }

    void Client::Run()
    {
        net::QuicClientConfig config =
        {
            ._config = {
                ._qlogDirectory = "./qlog/"
            },
            ._serverName = _config._ip.c_str(),
            ._port = 12345,
            ._alpn = Alpn,
        };

        _quic = std::make_unique<net::QuicClient>(config);
        _quic->Start();

        // 接続が切れたらすぐ終了する
        _quic->SetCallbackOnClose(
            [this](const std::shared_ptr<net::QuicConnection>& connection)
            {
                Stop();
            }
        );

        _connection = std::make_shared<ServerConnection>(this, _quic->GetConnection());

        _state = State::Lobby;
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
            if (auto system = _game.getServiceLocator()->Get<QuicControllerSystem>())
            {
                system->SetConnection(_connection->GetConnection());
                system->SetMyID(_connection->GetPlayerID());
            }
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

    void Client::OnReceiveSyncObject(const message_server_control::SyncPlayerAction& message)
    {
        _game.getServiceLocator()->Get<QuicControllerSystem>()->Receive(message);
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
