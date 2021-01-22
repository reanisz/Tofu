#pragma once

#include <entt/entt.hpp>
#include <tofu/utils.h>

namespace tofu::ball
{
	class PlayerController
	{
	public:
		PlayerController(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry);
		void Step();
	private:
		observer_ptr<ServiceLocator> _serviceLocator;
		observer_ptr<entt::registry> _registry;
	};

}

