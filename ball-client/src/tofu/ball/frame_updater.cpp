#include "tofu/ball/frame_updater.h"

#include "tofu/ball/input.h"
#include "tofu/ball/actions.h"
#include "tofu/ball/player_controller.h"
#include "tofu/ecs/physics.h"
#include "tofu/renderer/siv3d.h"
#include "tofu/ecs/box2d_primitive_renderer.h"

namespace tofu::ball
{
	UpdateSystem::UpdateSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry)
		: _serviceLocator(service_locator)
		, _registry(registry)
		, _thread(std::chrono::microseconds{ 16'666 }, [this](ScheduledUpdateThread&) { this->Step(); })
	{
	}

	void UpdateSystem::Start()
	{
		_thread.Start();
	}

	void UpdateSystem::Step()
	{
		_serviceLocator->Get<TickCounter>()->Step();
		_serviceLocator->Get<InputSystem>()->Step();
		
		auto tick = _serviceLocator->Get<TickCounter>()->GetCurrent();
		_serviceLocator->Get<ActionQueue>()->SetCurrentTick(tick);

		_serviceLocator->Get<PlayerController>()->Step();
		_serviceLocator->Get<ActionSystem>()->Step();

		_serviceLocator->Get<Physics>()->FollowTransform();
		_serviceLocator->Get<Physics>()->Step(1.f / 60);
		_serviceLocator->Get<Physics>()->WriteBackToTransform();
		
		_serviceLocator->Get<S3DRenderSystem>()->StartWrite();
		_serviceLocator->Get<Box2DPrimitiveRenderSystem>()->Render();

		{
			auto& input = _serviceLocator->Get<InputSystem>()->GetCurrent();
			auto pos = input._cursor.get();
			_serviceLocator->Get<S3DRenderSystem>()->Enqueue(render_command::S3DShapeFrame<Circle>{Circle{pos._x, pos._y, 5}}, 1);
		}
		_serviceLocator->Get<S3DRenderSystem>()->EndWrite();
	}

}
