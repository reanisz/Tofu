#pragma once

#include <entt/entt.hpp>
#include "tofu/utils.h"


namespace tofu
{
    using Angle = float;

    struct Transform {
        tVec2 _pos;
        Angle _angle;
    };

    using GameTick = StrongNumeric<class Tag_GameTick, std::uint32_t>;

    class TickCounter
    {
    public:
        TickCounter() noexcept
            : _now(0)
        {
        }
        GameTick GetCurrent() const noexcept
        {
            return _now;
        }
        void Step() noexcept
        {
            _now++;
        }

        void Set(GameTick tick) noexcept
        {
            _now = tick;
        }
    private:
        GameTick _now;
    };
}

