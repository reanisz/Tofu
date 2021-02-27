#include "tofu/ball/network.h"
#include "tofu/ball/net_server.h"

namespace tofu::ball
{
    ClientConnection::ClientConnection(observer_ptr<Server> server, const std::shared_ptr<net::QuicConnection>& quic, PlayerID player_id)
        : _server(server)
        , _quic(quic)
        , _id(player_id)
    {
    }

    void ClientConnection::Update()
    {
        switch (_state)
        {
        case State::WaitConnect:
            UpdateWaitConnect();
            return;
        case State::WaitJoinRequest:
            UpdateWaitJoinRequest();
            return;
        case State::Ingame:
            UpdateIngame();
            return;
        }
    }

    void ClientConnection::StartGame()
    {
        message_server_control::StartGame message;
        message._playerNum = 0;
        auto connections = _server->GetConnections();
        for (int i = 0; i < MaxPlayerNum; i++) 
        {
            strncpy(message._members[i]._name, connections[i]->GetName().c_str(), sizeof(message._members[i]._name));
            message._playerNum++;
        }
        SendMessage(_streamControlSend, message);

        _state = State::Ingame;
    }

	void ClientConnection::UpdateWaitConnect()
	{
        if (!_quic->IsConnected())
            return;

        _streamControlSend = _quic->OpenStream(ServerControlStreamId, false);
        _streamControlRecv = _quic->OpenStream(ClientControlStreamId, true);
        _state = State::WaitJoinRequest;
	}

    void ClientConnection::UpdateWaitJoinRequest()
    {
        auto [message, error] = ReadMessage<message_client_control::RequestJoin>(_streamControlRecv);

        if (error)
        {
            _error = error;
            return;
        }

        if (!message)
            return;

        _name = message->_userName;

        message_server_control::ApproveJoin approve;
        approve._playerId = *_id;
        _streamControlSend->Send(reinterpret_cast<std::byte*>(&approve), sizeof(approve));

        fmt::print("Joined Client: {} ({})\n", _name, *_id);

        _state = State::Ready;
    }

    void ClientConnection::UpdateIngame()
    {
        auto header = PeekHeader(_streamControlRecv);
        if (!header)
            return;
        switch (header->_messageType)
        {
        case message_client_control::SyncPlayerAction::message_type:
        {
            auto [message, error] = ReadMessage<message_client_control::SyncPlayerAction>(_streamControlRecv);
            if (message)
            {
                _server->OnReceiveSyncObject(*message);
                static std::size_t total = 0;
                total += sizeof(message);
                auto rtt = picoquic_get_rtt(_quic->GetRaw());
                fmt::print("recved sync obj[{}]({}): {}, {} <{}> RTT:{} \n", *message->_tick, *message->_player, message->_obj[0]._action.index(), message->_obj[1]._action.index(), total, rtt);
            }
        }
            break;
        }
    }

    void Server::Run()
    {
        net::QuicServerConfig config =
        {
            ._config = {
                ._qlogDirectory = "./qlog/",
                ._pingInterval = std::chrono::milliseconds{10}
            },
            ._port = 12345,
            ._certFile = { "./cert/_wildcard.reanisz.info+3.pem" },
            ._secretFile = { "./cert/_wildcard.reanisz.info+3-key.pem" },
            ._alpn = Alpn,
        };

        _quic = std::make_unique<net::QuicServer>(config);
        _quic->Start();

        _state = State::Lobby;

        // 1クライアントでも接続が切れたらすぐ終了する
        _quic->SetCallbackOnClose(
            [this](const std::shared_ptr<net::QuicConnection>& connection)
            {
                Stop();
            }
        );

        _quic->SetCallbackOnConnect(
            [this](const std::shared_ptr<net::QuicConnection>& connection)
            {
                if (MaxPlayerNum <= _clientNum)
                {
                    connection->Close();
                    return;
                }
                {
                    std::lock_guard lock{ _mutexConnection };
                    int id = _clientNum++;
                    _connections[id] = std::make_shared<ClientConnection>(this, connection, id);
                }
            }
        );

        while (!_end && _state == State::Lobby)
        {
            UpdateAtLobby();
            std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
        }

        InitGame();
        _game.start();

        while (!_end && _state == State::InGame)
        {
            UpdateAtIngame();
            std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
        }

        _quic->Exit();
    }
    void Server::Stop()
    {
        _end = true;
    }
    void Server::OnReceiveSyncObject(const message_client_control::SyncPlayerAction& message)
    {
        for (auto& connection : _connections)
        {
            if (connection->GetID() != message._player)
            {
                message_server_control::SyncPlayerAction send_msg =
                {
                    ._player = message._player,
                    ._tick = message._tick,
                    ._obj = message._obj,
                };

                connection->SendAsControl(send_msg);
            }
        }
    }
    void Server::UpdateAtLobby()
    {
        std::lock_guard lock{ _mutexConnection };
        bool ready = true;
        for (auto& client : _connections)
        {
            if (!client)
                continue;
            client->Update();
            if (client->GetState() != ClientConnection::State::Ready)
                ready = false;
        }
        if (_clientNum < MaxPlayerNum || !ready)
            return;

        for (auto& client : _connections)
        {
            if (!client)
                continue;
            client->StartGame();
        }

        _state = State::InGame;
    }

    void Server::UpdateAtIngame()
    {
        for (auto& client : _connections)
        {
            if (!client)
                continue;
            client->Update();
        }
        
        _game.update();
    }

    void Server::InitGame()
    {
        _game.initBaseSystems();
        _game.initEnitites();
    }
}

