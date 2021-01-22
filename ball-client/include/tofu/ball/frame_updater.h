#pragma once

#include <entt/entt.hpp>

#include <tofu/utils.h>
#include <tofu/containers.h>
#include <tofu/input.h>

#include <tofu/ecs/core.h>

namespace tofu::ball
{
	class UpdateSystem
	{
	public:
		UpdateSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry);

		void Start();

	private:
		void Step();

		observer_ptr<ServiceLocator> _serviceLocator;
		observer_ptr<entt::registry> _registry;

		ScheduledUpdateThread _thread;
	};
}
