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

	namespace jobs
	{
		class ProcessPlayerControl
		{
		public:
			ProcessPlayerControl(observer_ptr<PlayerController> system)
				: _system(system)
			{
			}

			void operator()() const
			{
				_system->Step();
			}

		private:
			observer_ptr<PlayerController> _system;
		};
	}

}

