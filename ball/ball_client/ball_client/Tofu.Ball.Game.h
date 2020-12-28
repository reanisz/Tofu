#pragma once

#include <variant>
#include <cstdint>

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

	struct GoalFrame
	{
		static std::tuple<entt::entity> Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, tVec2 pos);
	};

	struct Goal
	{
		entt::entity _frame;

		static std::tuple<entt::entity, Goal&> Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, tVec2 pos);
	};

	using GameTick = StrongNumeric<class Tag_GameTick, std::uint32_t>;

	namespace actions 
	{
		struct Dash 
		{
			tVec2 _target;
		};
		using Variant = std::variant<Dash>;
	}

	struct ActionCommand 
	{
		entt::entity _entity;
		actions::Variant _action;
		GameTick _tick;
	};

	class ActionQueue
	{
	public:
		void Enqueue(ActionCommand&& command) 
		{
			_queue.push_back(std::move(command));
		}

		std::vector<ActionCommand> Retrieve() 
		{
			std::vector<ActionCommand> ret;
			std::swap(ret, _queue);

			// NRVO(Named Return Value Optimization)‚ðŠú‘Ò
			return ret;
		}

	private:
		std::vector<ActionCommand> _queue;
	};


	class ActionSystem
	{
	public:
		ActionSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry);
		void Step();

	private:
		void apply(entt::entity entity, const actions::Dash& action);

		observer_ptr<ServiceLocator> _serviceLocator;
		observer_ptr<entt::registry> _registry;
	};

	struct Player
	{
		int _id;

		static std::tuple<entt::entity, Player&> Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, tVec2 pos);
	};

	class PlayerController
	{
	public:
		PlayerController(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry);
		void Step();
	private:
		observer_ptr<ServiceLocator> _serviceLocator;
		observer_ptr<entt::registry> _registry;
	};

	class TickCounter
	{
	public:
		TickCounter()
			: _now(0)
		{
		}
		GameTick GetCurrent() const 
		{
			return _now;
		}
		void Step() 
		{
			_now++;
		}

		void Overwrite(GameTick tick)
		{
			_now = tick;
		}
	private:
		GameTick _now;
	};


	class UpdateSystem 
	{
	public:
		UpdateSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry);
		void Step();
	private:
		observer_ptr<ServiceLocator> _serviceLocator;
		observer_ptr<entt::registry> _registry;
	};

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
