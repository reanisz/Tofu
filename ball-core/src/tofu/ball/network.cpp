#include "tofu/ball/network.h"

namespace tofu::ball
{
    std::optional<MessageHeader> PeekHeader(const std::shared_ptr<net::QuicStream>& stream)
    {
        MessageHeader header;
        if (stream->ReceivedSize() < sizeof(header))
            return std::nullopt;

        stream->Peek(reinterpret_cast<std::byte*>(&header), sizeof(header));
        return header;
    }
}