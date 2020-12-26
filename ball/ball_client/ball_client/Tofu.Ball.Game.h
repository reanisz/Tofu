#pragma once

#include <variant>

#include <entt/entt.hpp>
#include <Siv3D.hpp>

#include "Tofu.Utils.h"

namespace tofu::ball {
	struct Ball
	{
		float _radius;
	};

	struct Box
	{
	};

	struct Floor
	{
	};

	struct Player
	{
	};

	namespace actions 
	{
		struct Dash {
		};
		using Variant = std::variant<Dash>;
	}

	struct ActionCommand {
		actions::Variant _action;
		int _time;
	};


	class Game {
	public:
		Game();

		void run();

	private:

		void initialize();
		void game_loop();

	private:
		entt::registry _registry;
		ServiceLocator _serviceLocator;
		bool _end;
	};
}
