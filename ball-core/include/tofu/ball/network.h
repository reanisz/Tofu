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

	// サーバーがクライアントに投げる操作メッセージ (Reliable)
	inline constexpr net::StreamId ServerControlStreamId = 1;
	namespace message_server_control
	{
		inline constexpr const MessageType MessageTypeBase = 0b00'0000000;
		// 参加を許可
		struct ApproveJoin
		{
			static constexpr MessageType message_type = MessageTypeBase | 0x01;

			const MessageHeader _header = { sizeof(ApproveJoin), message_type };
			const std::uint8_t _messageType = MessageTypeBase | 0x01;
			std::uint8_t _playerId; // プレイヤーIDを通知
		};

		// 参加を拒絶。これを投げたら or 受け取ったら切断する
		struct RejectJoin
		{
			static constexpr MessageType message_type = MessageTypeBase | 0x02;

			const MessageHeader _header = { sizeof(RejectJoin), message_type };
		};

		// ゲーム終了
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

		// ゲームをはじめる
		struct StartGame
		{
			static constexpr MessageType message_type = MessageTypeBase | 0x04;

			const MessageHeader _header = { sizeof(StartGame), message_type };
		};
	}

	// クライアントがサーバーに投げる操作メッセージ (Reliable)
	inline constexpr net::StreamId ClientControlStreamId = 2;
	namespace message_client_control
	{
		inline constexpr const MessageType MessageTypeBase = 0b01'0000000;

		// 部屋に参加することを要請する。 接続したらまずこれを投げる
		struct RequestJoin
		{
			static constexpr MessageType message_type = MessageTypeBase | 0x01;

			const MessageHeader _header = { sizeof(RequestJoin), message_type };
			char _userName[30];
		};
		// 32byteになってるか不安
		static_assert(sizeof(RequestJoin) == 32);


	}

}

