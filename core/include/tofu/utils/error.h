#pragma once

#include <fmt/core.h>
#include <optional>

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
            fmt::print("message=<{}>, file={}, line={}\n", _message, _file, _line);
        }
	};

    using Error = std::optional<ErrorData>;

#if _DEBUG
#define TOFU_MAKE_ERROR(...) ErrorData{fmt::format(__VA_ARGS__), __FILE__, __LINE__}
#else
#define TOFU_MAKE_ERROR(...) ErrorData{fmt::format(__VA_ARGS__)}
#endif
}

