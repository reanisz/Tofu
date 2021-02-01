#pragma once

#include <type_traits>
#include <tofu/net/quic.h>

#include <tofu/net/completely_sync.h>
#include "tofu/ball/actions.h"
#include "tofu/ball/network.h"

#undef SendMessage

namespace tofu::ball
{
	// �V���A���C�Y�����o�C�g������̂܂ܒʐM���Ă�̂ŁA�g���r�A���R�s�[�\�Ȍ^�łȂ���Έ��S�ɑ���M�ł��Ȃ�
	static_assert(std::is_trivially_copyable_v<SyncObject>);

	inline constexpr std::uint32_t SyncWindowSize = 16;

	class CompletelySyncSystem : public tofu::net::CompletelySyncSystem<SyncObject, SyncWindowSize>
	{
	public:
		CompletelySyncSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, std::size_t player_num, std::uint32_t default_delay)
			: tofu::net::CompletelySyncSystem<SyncObject, SyncWindowSize>(player_num, default_delay)
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
			SyncObject _obj;
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

		// �f�[�^����M���Ĉ�U�o�b�t�@�ɒ��߂�
		void Receive(const message_server_control::SyncPlayerAction& message);
		void Receive(const message_client_control::SyncPlayerAction& message);

		// �o�b�t�@�ɗ��܂��Ă���f�[�^��SyncSystem�ɋl�߂�. 1�t���[����1�x�s��
		void ApplySyncObject();
	private:
		std::shared_ptr<tofu::net::QuicConnection> _quic;
		std::shared_ptr<tofu::net::QuicStream> _sendStream;
		PlayerID _playerId;

        observer_ptr<ServiceLocator> _serviceLocator;
        observer_ptr<entt::registry> _registry;

		// ���t���[���œK�p����\���SyncObject
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