#pragma once

#include "tofu/utils.h"

namespace tofu::input
{
	class Button
	{
	private:
		struct State 
		{
			bool _pressed = false;
		};
	public:

		constexpr Button()
		{
			for (int i = 0; i < _state.capacity(); i++) 
			{
				_state.push_back({});
			}
		}

		constexpr void NewFrame()
		{
			_state.pop_front();
			_state.push_back({});
		}

		constexpr void press()
		{
			_state[0]._pressed = true;
		}

		constexpr bool is_pressed() const noexcept
		{
			return _state[0]._pressed;
		}

		constexpr bool is_released() const noexcept
		{
			return !_state[0]._pressed;
		}

		constexpr bool is_down() const noexcept
		{
			return !_state[-1]._pressed && _state[0]._pressed;
		}
		constexpr bool is_up() const noexcept
		{
			return _state[-1]._pressed && !_state[0]._pressed;
		}

	private:
		RingBuffer<State, 2, ring_buffer::TailOrigin> _state;
	};
	class Axis2
	{
	private:
		struct State 
		{
			tVec2 _vec;
		};
	public:

		constexpr Axis2()
		{
			for (int i = 0; i < _state.capacity(); i++) 
			{
				_state.push_back({});
			}
		}

		constexpr void NewFrame()
		{
			_state.pop_front();
			_state.push_back({});
		}

		constexpr void set(tVec2 vec) noexcept
		{
			_state[0]._vec = vec;
		}

		constexpr tVec2 get() const noexcept
		{
			return _state[0]._vec;
		}

		constexpr bool is_moved() const noexcept
		{
			return _state[-1]._vec == _state[0]._vec;
		}

		constexpr tVec2 difference() const noexcept
		{
			return _state[0]._vec - _state[-1]._vec;
		}

	private:
		RingBuffer<State, 2, ring_buffer::TailOrigin> _state;
	};
}
