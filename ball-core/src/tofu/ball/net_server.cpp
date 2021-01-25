#include "tofu/ball/network.h"
#include "tofu/ball/net_server.h"

namespace tofu::ball
{
    void Server::Run()
    {
        net::QuicServerConfig config =
        {
            ._config = {
                ._qlogDirectory = "./qlog/"
            },
            ._port = 8000,
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
                if (ClientMax <= _clientNum)
                {
                    connection->Close();
                    return;
                }
                {
                    std::lock_guard lock{ _mutexConnection };
                    int id = _clientNum++;
                    _connections[id] = std::make_shared<ClientConnection>(connection, id);
                }
            }
        );

        while (!_end && _state == State::Lobby)
        {
            UpdateAtLobby();
            std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
        }

        _quic->Exit();
    }
    void Server::Stop()
    {
        _end = true;
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
        if (_clientNum < ClientMax)
            return;
    }

    void Server::InitGame()
    {
        _game.initBaseSystems();
        _game.initEnitites();
    }

    ClientConnection::ClientConnection(const std::shared_ptr<net::QuicConnection>& quic, PlayerID player_id)
        : _quic(quic)
        , _id(player_id)
    {
        _streamControlSend = quic->OpenStream(ServerControlStreamId);
        _streamControlRecv = quic->OpenStream(ClientControlStreamId);
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
        }
    }

	void ClientConnection::UpdateWaitConnect()
	{
        if (_quic->IsConnected())
            _state = State::WaitJoinRequest;
	}

    void ClientConnection::UpdateWaitJoinRequest()
    {
        auto header = PeekHeader(_streamControlRecv);
        if (!header)
            return;

        if (header->_messageType != message_client_control::RequestJoin::message_type)
        {
            _error = TOFU_MAKE_ERROR("Received invalid message. type=({})", header->_messageType);
            return;
        }

        if (_streamControlRecv->ReceivedSize() < header->_packetSize)
            return;

        message_client_control::RequestJoin message;
        _streamControlRecv->Read(reinterpret_cast<std::byte*>(&message), sizeof(message));

        _name = message._userName;

        message_server_control::ApproveJoin approve;
        approve._playerId = *_id;
        _streamControlSend->Send(reinterpret_cast<std::byte*>(&approve), sizeof(approve));

        _state = State::Ready;
    }
}

