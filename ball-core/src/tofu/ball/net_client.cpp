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
        auto header = PeekHeader(_streamControlRecv);
        if (!header)
            return;

        if (header->_messageType != message_server_control::ApproveJoin::message_type)
        {
            _error = TOFU_MAKE_ERROR("Received invalid message. type=({})", header->_messageType);
            return;
        }

        if (_streamControlRecv->ReceivedSize() < header->_messageType)
            return;

        message_server_control::ApproveJoin message;
        _streamControlRecv->Read(reinterpret_cast<std::byte*>(&message), sizeof(message));

        _id = message._playerId;
        _state = State::Ready;
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

        while (!_end && _state == State::Lobby)
        {
            UpdateAtLobby();
            std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
        }

        _quic->Exit();
    }
    void Client::Stop()
    {
        _end = true;
    }
    void Client::UpdateAtLobby()
    {
        _connection->Update();
    }
}
