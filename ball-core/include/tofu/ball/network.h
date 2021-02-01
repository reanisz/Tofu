#pragma once

#include <type_traits>
#include <optional>

#include <tofu/net/quic.h>
#undef SendMessage

#include <tofu/ball/player.h>
#include <tofu/ball/actions.h>

namespace tofu::ball
{
	struct SyncObject
	{
        actions::Variant _action;
	};

	inline constexpr const char* Alpn = "tofu_ball";

	using PacketSize = std::uint8_t;
	using MessageType = std::uint8_t;

	struct MessageHeader
	{
		PacketSize _packetSize;
		PacketSize _messageType;
	};

	std::optional<MessageHeader> PeekHeader(const std::shared_ptr<net::QuicStream>& stream);

	inline constexpr const int MaxPlayerNum = 2;
	
	template<class T> 
	std::tuple<std::optional<T>, tofu::Error> ReadMessage(const std::shared_ptr<net::QuicStream>& stream)
	{
		// �V���A���C�Y�����o�C�g������̂܂ܒʐM���Ă�̂ŁA�g���r�A���R�s�[�\�Ȍ^�łȂ���Έ��S�ɑ���M�ł��Ȃ�
		static_assert(std::is_trivially_copyable_v<T>);

        auto header = PeekHeader(stream);
        if (!header)
			return { std::nullopt, std::nullopt };

        if (header->_messageType != T::message_type)
        {
			return { std::nullopt, TOFU_MAKE_ERROR("Received invalid message. type=({})", header->_messageType) };
        }

        if (stream->ReceivedSize() < header->_packetSize)
			return { std::nullopt, std::nullopt };

        T message;
        stream->Read(reinterpret_cast<std::byte*>(&message), sizeof(message));

		return { message, std::nullopt };
	}

	template<class T> 
	void SendMessage(const std::shared_ptr<net::QuicStream>& stream, const T& message)
	{
		// �V���A���C�Y�����o�C�g������̂܂ܒʐM���Ă�̂ŁA�g���r�A���R�s�[�\�Ȍ^�łȂ���Έ��S�ɑ���M�ł��Ȃ�
		static_assert(std::is_trivially_copyable_v<T>);
		
        stream->Send(reinterpret_cast<const std::byte*>(&message), sizeof(message));
	}

	// �T�[�o�[���N���C�A���g�ɓ����鑀�상�b�Z�[�W (Reliable)
	inline constexpr net::StreamId ServerControlStreamId = 1;
	namespace message_server_control
	{
		inline constexpr const MessageType MessageTypeBase = 0b00'0000000;
		// �Q��������
		struct ApproveJoin
		{
			static constexpr MessageType message_type = MessageTypeBase | 0x01;

			const MessageHeader _header = { sizeof(ApproveJoin), message_type };
			const std::uint8_t _messageType = MessageTypeBase | 0x01;
			std::uint8_t _playerId; // �v���C���[ID��ʒm
		};

		// �Q��������B����𓊂����� or �󂯎������ؒf����
		struct RejectJoin
		{
			static constexpr MessageType message_type = MessageTypeBase | 0x02;

			const MessageHeader _header = { sizeof(RejectJoin), message_type };
		};

		// �Q�[���I��
		struct CloseServer
		{
			static constexpr MessageType message_type = MessageTypeBase | 0x03;
			enum class Reason : std::uint8_t
			{
				EndGame,
				DisconnectClient,
			};

			const MessageHeader _header = { sizeof(CloseServer), message_type };
			Reason _reason;
		};

		// �Q�[�����͂��߂�
		struct StartGame
		{
			static constexpr MessageType message_type = MessageTypeBase | 0x04;
			struct MemberInfo
			{
				PlayerID::value_type _id;
				char _name[30];
			};

			const MessageHeader _header = { sizeof(StartGame), message_type };

			std::uint8_t _playerNum;
			MemberInfo _members[MaxPlayerNum];
		};

		// �v���C���[�A�N�V���������N���C�A���g�ɓ`����
		struct SyncPlayerAction
		{
			static constexpr MessageType message_type = MessageTypeBase | 0x05;
			const MessageHeader _header = { sizeof(SyncPlayerAction), message_type };

			PlayerID _player;
			GameTick _tick;
			SyncObject _obj;
		};
	}

	// �N���C�A���g���T�[�o�[�ɓ����鑀�상�b�Z�[�W (Reliable)
	inline constexpr net::StreamId ClientControlStreamId = 2;
	namespace message_client_control
	{
		inline constexpr const MessageType MessageTypeBase = 0b01'0000000;

		// �����ɎQ�����邱�Ƃ�v������B �ڑ�������܂�����𓊂���
		struct RequestJoin
		{
			static constexpr MessageType message_type = MessageTypeBase | 0x01;

			const MessageHeader _header = { sizeof(RequestJoin), message_type };
			char _userName[30];
		};
		// 32byte�ɂȂ��Ă邩�s��
		static_assert(sizeof(RequestJoin) == 32);

		// �����̃A�N�V���������T�[�o�[�ɓ`����
		struct SyncPlayerAction
		{
			static constexpr MessageType message_type = MessageTypeBase | 0x02;
			const MessageHeader _header = { sizeof(SyncPlayerAction), message_type };

			PlayerID _player;
			GameTick _tick;
			SyncObject _obj;
		};

	}

}

