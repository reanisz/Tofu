#include "tofu/ball/actions.h"

#include <tofu/ecs/physics.h>

namespace tofu::ball 
{
	ActionSystem::ActionSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry)
		: _serviceLocator(service_locator)
		, _registry(registry)
	{
	}

	void ActionSystem::Step()
	{
		auto actions = _serviceLocator->Get<ActionQueue>()->Retrieve();
		for (auto& action : actions)
		{
			std::visit([this, entity = action._entity](const auto& v) { this->apply(entity, v); }, action._action);
		}
	}

	void ActionSystem::apply(entt::entity entity, const actions::Move& action)
	{
		auto body = _registry->get<RigidBody>(entity)._body;
		auto d = action._target - tVec2{ body->GetPosition() };
		d.Normalize();
		body->ApplyForceToCenter(d * 0.7f, true);
	}

	void ActionSystem::apply(entt::entity entity, const actions::Dash& action)
	{
		auto body = _registry->get<RigidBody>(entity)._body;
		auto d = action._target - tVec2{ body->GetPosition() };
		d.Normalize();
		body->SetLinearVelocity({ 0, 0 });
		body->ApplyForceToCenter(d * 40, true);
	}
}

