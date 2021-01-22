#include "tofu/ball/player_controller.h"

#include "tofu/ecs/core.h"
#include "tofu/ball/input.h"
#include "tofu/ball/actions.h"
#include "tofu/ball/player.h"

#include "tofu/ecs/box2d_primitive_renderer.h"

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

		for (auto&& [entity, player] : _registry->view<Player>().proxy()) 
		{
			auto queue = _serviceLocator->Get<ActionQueue>();
			auto clock = _serviceLocator->Get<TickCounter>();
			auto tick = clock->GetCurrent() + GameTick{ 0 };
			auto scale = _serviceLocator->Get<Box2DPrimitiveRenderSystem>()->GetScale();
			auto target = (1 / scale) * input._cursor.get();
			if (input._leftClick.is_down()) {
				queue->Enqueue({ entity, actions::Dash{ target }, tick });
			}
			if (input._rightClick.is_pressed()) {
				queue->Enqueue({ entity, actions::Move{ target }, tick });
			}

		}
	}
}
