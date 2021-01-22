#pragma once

#include <tuple>

#include <entt/entt.hpp>
#include <tofu/utils.h>

namespace tofu::ball
{
	struct Player
	{
		int _id;

		static std::tuple<entt::entity, Player&> Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, int id, tVec2 pos);
	};
}
