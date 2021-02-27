#include "tofu/ball/sync.h"

namespace tofu::ball
{
    QuicControllerSystem::QuicControllerSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry)
        : _serviceLocator(service_locator)
        , _registry(registry)
    {
    }
    void QuicControllerSystem::SetConnection(const std::shared_ptr<tofu::net::QuicConnection>& quic)
    {
        _quic = quic;
        _sendStream = quic->GetStream(ClientControlStreamId);
    }
    void QuicControllerSystem::Receive(const message_server_control::SyncPlayerAction& message)
    {
        std::lock_guard lock{_syncObjectMutex};
        _syncObjectBuffer.push_back(message);
    }
    void QuicControllerSystem::Receive(const message_client_control::SyncPlayerAction& message)
    {
        std::lock_guard lock{_syncObjectMutex};
        _syncObjectBuffer.push_back(message);
    }
    void QuicControllerSystem::ApplySyncObject()
    {
        std::lock_guard lock{_syncObjectMutex};

        auto sync = _serviceLocator->Get<CompletelySyncSystem>();
        auto current_tick = _serviceLocator->Get<TickCounter>()->GetCurrent();
        for (auto& obj : _syncObjectBuffer) 
        {
            auto tick_after = obj._tick - current_tick;
            for (std::uint32_t i = 0; i < SyncWindowSize; i++)
            {
                sync->SetData(*(obj._player), tick_after + GameTick{ i }, obj._obj[i]);
            }
        }
        _syncObjectBuffer.clear();
    }
}