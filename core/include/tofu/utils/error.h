#pragma once

#include <fmt/core.h>
#include <string>
#include <optional>

#ifdef TOFU_ENABLE_SIV3D
#define TOFU_FMT fmt_s3d
#else
#define TOFU_FMT fmt
#endif

namespace tofu
{
    struct ErrorData
    {
        std::string _message;
#if _DEBUG
        const char* _file;
        int _line;
#endif

        void Dump() const
        {
#if _DEBUG
            TOFU_FMT::print("message=<{}>, file={}, line={}\n", _message, _file, _line);
#else
            TOFU_FMT::print("message=<{}>\n", _message);
#endif
        }
    };

    using Error = std::optional<ErrorData>;

#if _DEBUG
#define TOFU_MAKE_ERROR(...) ErrorData{TOFU_FMT::format(__VA_ARGS__), __FILE__, __LINE__}
#else
#define TOFU_MAKE_ERROR(...) ErrorData{TOFU_FMT::format(__VA_ARGS__)}
#endif
}

