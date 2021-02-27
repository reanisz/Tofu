#pragma once

#include <type_traits>
#include <tofu/net/quic.h>

#include <tofu/net/completely_sync.h>
#include "tofu/ball/actions.h"
#include "tofu/ball/network.h"

#undef SendMessage

namespace tofu::ball
{
	// シリアライズせずバイト列をそのまま通信してるので、トリビアルコピー可能な型でなければ安全に送受信できない
	static_assert(std::is_trivially_copyable_v<SyncObject>);

	inline constexpr std::uint32_t SyncBufferSize = 8;

	class CompletelySyncSystem : public tofu::net::CompletelySyncSystem<SyncObject, SyncBufferSize>
	{
	public:
		CompletelySyncSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, std::size_t player_num, std::uint32_t default_delay)
			: tofu::net::CompletelySyncSystem<SyncObject, SyncBufferSize>(player_num, default_delay)
			, _serviceLocator(service_locator)
			, _registry(registry)
		{
		}

		void ApplyToActionQueue()
		{
			assert(CanStep());

			auto tick = _serviceLocator->Get<TickCounter>()->GetCurrent();
			auto action_queue = _serviceLocator->Get<ActionQueue>();
			auto data = Top();
			for (int i = 0; i < MaxPlayerNum; i++)
			{
				auto res_find = Player::Find(_registry, i);
				assert(res_find);

				action_queue->Enqueue(tofu::ball::ActionCommand{
					._entity = std::get<0>(*res_find),
					._action = data[i]->_action,
					._tick = tick,
					});
			}
		}

	private:
		observer_ptr<entt::registry> _registry;
		observer_ptr<ServiceLocator> _serviceLocator;
	};

	class QuicControllerSystem
	{
		struct SyncMessage
		{
			SyncMessage() = default;
			SyncMessage(const message_server_control::SyncPlayerAction& message)
				: _player(message._player)
				, _tick(message._tick)
				, _obj(message._obj)
			{
			}
			SyncMessage(const message_client_control::SyncPlayerAction& message)
				: _player(message._player)
				, _tick(message._tick)
				, _obj(message._obj)
			{
			}

			PlayerID _player;
			GameTick _tick;
			std::array<SyncObject, SyncWindowSize> _obj;
		};
	public:
        QuicControllerSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry);

		void SetConnection(const std::shared_ptr<tofu::net::QuicConnection>& quic);

		template<class T>
		void Send(const T& message)
		{
			tofu::ball::SendMessage(_sendStream, message);
		}
		
		void SetMyID(PlayerID id)
		{
			_playerId = id;
		}
		PlayerID GetMyID() const noexcept
		{
			return _playerId;
		}

		// データを受信して一旦バッファに貯める
		void Receive(const message_server_control::SyncPlayerAction& message);
		void Receive(const message_client_control::SyncPlayerAction& message);

		// バッファに溜まっているデータをSyncSystemに詰める. 1フレームに1度行う
		void ApplySyncObject();
	private:
		std::shared_ptr<tofu::net::QuicConnection> _quic;
		std::shared_ptr<tofu::net::QuicStream> _sendStream;
		PlayerID _playerId;

        observer_ptr<ServiceLocator> _serviceLocator;
        observer_ptr<entt::registry> _registry;

		// 次フレームで適用する予定のSyncObject
		std::mutex _syncObjectMutex;
		std::vector<SyncMessage> _syncObjectBuffer;
	};

	namespace job_conditions
	{
		struct IsStepable {};
	}
	
    namespace jobs
    {
		class CheckStepable
		{
		public:
			CheckStepable(observer_ptr<CompletelySyncSystem> system)
				: _system(system)
			{
			}

			std::optional<condition_tag> operator()() const
			{
				if (_system->CanStep())
				{
					return get_condition_tag<job_conditions::IsStepable>();
				}
				else
				{
					return std::nullopt;
				}
			}

		private:
			observer_ptr<CompletelySyncSystem> _system;
		};

		class ApplySyncBufferToActionQueue
		{
		public:
			ApplySyncBufferToActionQueue(observer_ptr<CompletelySyncSystem> system)
				: _system(system)
			{
			}

			void operator()() const
			{
				_system->ApplyToActionQueue();
			}

		private:
			observer_ptr<CompletelySyncSystem> _system;
		};

		class StepSyncBuffer
		{
		public:
			StepSyncBuffer(observer_ptr<CompletelySyncSystem> system)
				: _system(system)
			{
			}

			void operator()() const
			{
				_system->Step();
			}

		private:
			observer_ptr<CompletelySyncSystem> _system;
		};

        class ApplySyncObject
        {
        public:
            ApplySyncObject(observer_ptr<QuicControllerSystem> system)
                : _system(system)
            {
            }

            void operator()() const
            {
                _system->ApplySyncObject();
            }

        private:
            observer_ptr<QuicControllerSystem> _system;
        };
    }
}
