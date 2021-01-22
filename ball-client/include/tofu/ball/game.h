#pragma once

#include <variant>
#include <cstdint>
#include <thread>
#include <condition_variable>
#include <optional>

#include <entt/entt.hpp>
#include <Siv3D.hpp>

#include <tofu/utils.h>
#include <tofu/containers.h>
#include <tofu/input.h>

namespace tofu::ball {
	struct InputState
	{
		constexpr void NewFrame()
		{
			assert(!_deployed);
			if (_deployed)
				return;

			_cursor.NewFrame();
			_leftClick.NewFrame();
			_rightClick.NewFrame();
		}

		void Update()
		{
			assert(!_deployed);
			if (_deployed)
				return;

			_cursor.set(Cursor::PosF());
			if (MouseL.pressed())
				_leftClick.press();
			if (MouseR.pressed())
				_rightClick.press();
		}

		InputState Deploy()
		{
			auto ret = *this;
			ret._deployed = true;
			return ret;
		}

		input::Axis2 _cursor;
		input::Button _leftClick;
		input::Button _rightClick;

	private:
		bool _deployed = false;
	};

	class InputSystem
	{
	public:
		InputSystem()
		{
		}

		// NOTE: Step()とスレッドセーフな操作ではないことに注意
		const InputState& GetCurrent() const
		{
			return _currentState;
		}

		// 今フレームの入力を確定し、次フレームの準備をする
		// UpdateThread側で呼び出すことが想定されている
		//   NOTE: GetCurrent()とスレッドセーフな操作ではないことに注意
		void Step()
		{
			std::lock_guard lock{ _mutex };
			_currentState = _state.Deploy();
			_state.NewFrame();
		}

		// 今フレームの入力を処理する。
		// MainThread側で呼び出すことが想定されている
		void Update()
		{
			std::lock_guard lock{ _mutex };
			_state.Update();
		}

	private:
		std::mutex _mutex;
		InputState _state;

		InputState _currentState;
	};

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
		struct Move
		{
			tVec2 _target;
		};
		struct Dash 
		{
			tVec2 _target;
		};
		using Variant = std::variant<Move, Dash>;
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
		static constexpr int QueueCount = 4;

		void SetCurrentTick(GameTick tick) noexcept
		{
			_current = tick;
		}

		void Enqueue(ActionCommand&& command) 
		{
			auto tick = command._tick;
			auto d = tick - _current;
			if (d < _queues.size()) {
				_queues[*d].push_back(std::move(command));
			}
			else {
				_futureActions.push_back(command);
			}
		}

		std::vector<ActionCommand> Retrieve() 
		{
			std::vector<ActionCommand> ret;
			std::swap(ret, _queues[0]);

			for (int i = 0; i < _queues.size() - 1; i++) {
				std::swap(_queues[i], _queues[i + 1]);
			}

			auto& left = _queues[_queues.size() - 1];
			auto t = *_current + QueueCount - 1;
			for (auto& act : _futureActions) {
				if (act._tick == t)
					left.push_back(act);
			}
			std::erase_if(_futureActions, [t](const ActionCommand& v) { return v._tick == t; });

			// NRVO(Named Return Value Optimization)を期待
			return ret;
		}

	private:
		GameTick _current;

		// _queues[x] : x Tick先に処理するAction
		std::array<std::vector<ActionCommand>, QueueCount> _queues;

		// queuesに溢れるくらい未来のaction
		std::vector<ActionCommand> _futureActions;
	};


	class ActionSystem
	{
	public:
		ActionSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry);
		void Step();

	private:
		void apply(entt::entity entity, const actions::Move& action);
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
		TickCounter() noexcept
			: _now(0)
		{
		}
		GameTick GetCurrent() const noexcept
		{
			return _now;
		}
		void Step() noexcept
		{
			_now++;
		}

		void Set(GameTick tick) noexcept
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

		void Start();

	private:
		void Step();

		observer_ptr<ServiceLocator> _serviceLocator;
		observer_ptr<entt::registry> _registry;

		ScheduledUpdateThread _thread;
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
