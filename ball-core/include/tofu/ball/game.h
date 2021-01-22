#pragma once

#include <variant>
#include <cstdint>
#include <thread>
#include <condition_variable>
#include <optional>

#include <entt/entt.hpp>

#include <tofu/utils.h>
#include <tofu/containers.h>
#include <tofu/input.h>

#include <tofu/ecs/core.h>

namespace tofu::ball 
{
	class Game {
	public:
		Game();

		void initBaseSystems();
		void initEnitites();

		void start();
		void update();

		observer_ptr<entt::registry> getRegistry();
		observer_ptr<ServiceLocator> getServiceLocator();

	private:
		void initSystems();
		void initStage();
		void initPlayers();
		void initBall();

	private:
		entt::registry _registry;
		ServiceLocator _serviceLocator;
	};
}
