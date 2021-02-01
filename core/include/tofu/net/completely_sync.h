#pragma once

#include <optional>
#include <vector>
#include <tofu/ecs/core.h>
#include <tofu/containers/ring_buffer.h>

namespace tofu::net
{
	// 完全同期方式でゲームをStepするシステム
	//  TODO: テストを書く
	template<class TSyncType, std::uint32_t BufferSize>
	class CompletelySyncSystem
	{
		struct State
		{
			constexpr State(std::size_t player_num)
				: _state(player_num)
			{
			}

			constexpr bool HasAllPlayerData() const noexcept
			{
				for (auto& state : _state)
				{
					if (!state)
						return false;
				}
				return true;
			}

			std::vector<std::optional<TSyncType>> _state;
		};

	public:
		CompletelySyncSystem(std::size_t player_num, std::uint32_t default_delay)
			: _playerNum(player_num)
		{
			for (std::uint32_t i = 0; i < BufferSize; i++) 
			{
				_buffer.push_back(State{ player_num });
			}
			for (std::uint32_t i = 0; i < default_delay; i++) 
				for (std::size_t p = 0; p < player_num; p++)
					_buffer[i]._state[p] = TSyncType{};
		}

		bool CanStep() const noexcept
		{
			return _buffer[0].HasAllPlayerData();
		}

		bool HasData(GameTick tick_after, std::size_t player_id) const noexcept
		{
			assert(tick_after < BufferSize);
			return _buffer[*tick_after]._state[player_id];
		}

		const std::vector<std::optional<TSyncType>>& Top() const noexcept
		{
			return _buffer[0]._state;
		}

		void Step() noexcept
		{
			_buffer.pop_front();
			_buffer.push_back(State{ _playerNum });
		}

		void SetData(std::size_t player_id, GameTick tick_after, const TSyncType& data) noexcept
		{
			assert(tick_after < BufferSize);
			_buffer[*tick_after]._state[player_id] = data;
		}

	private:
		std::size_t _playerNum;
		RingBuffer<State, BufferSize> _buffer;
		GameTick _nextTick = 0;
	};

}
