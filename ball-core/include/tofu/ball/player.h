#pragma once

#include <tuple>

#include <entt/entt.hpp>
#include <tofu/utils.h>

namespace tofu::ball
{
    using PlayerID = StrongNumeric<class tag_PlayerID, std::uint8_t>;

    struct Player
    {
        PlayerID _id;

        static std::tuple<entt::entity, Player&> Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, int id, tVec2 pos);
        static std::optional<std::tuple<entt::entity, Player&>> Find(observer_ptr<entt::registry> registry, PlayerID id);
    };
}
