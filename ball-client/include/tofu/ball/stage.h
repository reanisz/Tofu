#pragma once

#include <tuple>

#include <entt/entt.hpp>
#include <tofu/utils.h>

namespace tofu::ball
{
	struct Ball
	{
		float _radius;
	};

	struct Box
	{
	};

	struct Floor
	{
	};

	struct GoalFrame
	{
		static std::tuple<entt::entity> Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, tVec2 pos);
	};

	struct Goal
	{
		entt::entity _frame;

		static std::tuple<entt::entity, Goal&> Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, tVec2 pos);
	};

	void generate_stage(observer_ptr<tofu::ServiceLocator> service_locator, observer_ptr<entt::registry> registry);
}

