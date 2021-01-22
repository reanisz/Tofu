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

namespace tofu::ball 
{
	// 1フレームのユーザー入力データ
	struct InputState
	{
		constexpr void NewFrame() noexcept
		{
			assert(!_deployed);
			if (_deployed)
				return;

			_cursor.NewFrame();
			_leftClick.NewFrame();
			_rightClick.NewFrame();
		}

		constexpr void Update() noexcept
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

		constexpr InputState Deploy() noexcept
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

	// ユーザー入力を管理しゲームに引き渡すひと
	class InputSystem
	{
	public:
		InputSystem() noexcept
		{
		}

		// NOTE: Step()とスレッドセーフな操作ではないことに注意
		constexpr const InputState& GetCurrent() const noexcept
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
}
