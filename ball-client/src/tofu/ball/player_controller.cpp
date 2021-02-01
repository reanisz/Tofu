#include "tofu/ball/player_controller.h"

#include "tofu/ecs/core.h"
#include "tofu/ball/input.h"
#include "tofu/ball/actions.h"
#include "tofu/ball/player.h"

#include "tofu/ecs/box2d_primitive_renderer.h"

#include "tofu/ball/sync.h"

namespace tofu::ball
{
    PlayerController::PlayerController(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry)
        : _serviceLocator(service_locator)
        , _registry(registry)
    {
    }

    void PlayerController::Step()
    {
        auto& input = _serviceLocator->Get<InputSystem>()->GetCurrent();

        SyncObject sync;
        auto clock = _serviceLocator->Get<TickCounter>();
        auto tick_after = ActionDelay;

        for (auto&& [entity, player] : _registry->view<Player>().proxy()) 
        {
            auto queue = _serviceLocator->Get<ActionQueue>();
            auto scale = _serviceLocator->Get<Box2DPrimitiveRenderSystem>()->GetScale();
            auto target = (1 / scale) * input._cursor.get();
            if (input._leftClick.is_down()) {
                sync._action = actions::Dash{ target };
            }
            if (input._rightClick.is_pressed()) {
                sync._action = actions::Move{ target };
            }
        }

        PlayerID id = 0;
        auto net_system = _serviceLocator->Get<QuicControllerSystem>();
        if (net_system)
        {
            id = net_system->GetMyID();
        }
        auto sync_system = _serviceLocator->Get<CompletelySyncSystem>();

        sync_system->SetData(*id, tick_after, sync);
        if (net_system)
        {
            message_client_control::SyncPlayerAction message = {
                ._player = *id,
                ._tick = tick_after,
                ._obj = sync,
            };
            net_system->Send(message);
        }
        
    }
}
