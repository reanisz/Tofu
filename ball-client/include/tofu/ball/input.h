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
	// 1�t���[���̃��[�U�[���̓f�[�^
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

	// ���[�U�[���͂��Ǘ����Q�[���Ɉ����n���Ђ�
	class InputSystem
	{
	public:
		InputSystem() noexcept
		{
		}

		// NOTE: Step()�ƃX���b�h�Z�[�t�ȑ���ł͂Ȃ����Ƃɒ���
		constexpr const InputState& GetCurrent() const noexcept
		{
			return _currentState;
		}

		// ���t���[���̓��͂��m�肵�A���t���[���̏���������
		// UpdateThread���ŌĂяo�����Ƃ��z�肳��Ă���
		//   NOTE: GetCurrent()�ƃX���b�h�Z�[�t�ȑ���ł͂Ȃ����Ƃɒ���
		void Step()
		{
			std::lock_guard lock{ _mutex };
			_currentState = _state.Deploy();
			_state.NewFrame();
		}

		// ���t���[���̓��͂���������B
		// MainThread���ŌĂяo�����Ƃ��z�肳��Ă���
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
