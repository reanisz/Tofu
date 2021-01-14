#pragma once

#include <filesystem>
#include <optional>
#include <thread>

#include <tofu/utils/strong_numeric.h>
#include <tofu/utils/observer_ptr.h>
#include <tofu/utils/error.h>

#include <picoquic.h>
#include <picoquic_packet_loop.h>

namespace tofu::net
{
    namespace error_code
    {
        // Tofu固有エラーコードの基点
        inline constexpr int Base = 0x10000000;

        // ユーザー操作によって中断された
        inline constexpr int Interrupt = Base + 1;

        // コンテキストが見つからなかった
        inline constexpr int ContextNotFound = Base + 2;
    }

    using Port = StrongNumeric<class tag_Port, int>;

    struct QuicConfig
    {
        std::filesystem::path _qlogDirectory;
        int _qlogLevel = 1;
    };
}

