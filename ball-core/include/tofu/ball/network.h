#pragma once

#include <optional>

#include <tofu/net/quic.h>

namespace tofu::ball
{
	inline constexpr const char* Alpn = "tofu_ball";

	using PacketSize = std::uint8_t;
	using MessageType = std::uint8_t;

	struct MessageHeader
	{
		PacketSize _packetSize;
		PacketSize _messageType;
	};

	std::optional<MessageHeader> PeekHeader(const std::shared_ptr<net::QuicStream>& stream);

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

			const MessageHeader _header = { sizeof(StartGame), message_type };
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


	}

}

