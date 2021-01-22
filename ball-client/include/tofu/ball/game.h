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

		void run();

	private:
		void initialize();

		void initSystems();
		void initStage();
		void initPlayers();
		void initBall();

		void game_loop();

	private:
		entt::registry _registry;
		ServiceLocator _serviceLocator;
		bool _end;
	};
}
