#pragma once

#include <box2d/box2d.h>

#include "Tofu.Utils.h"
#include "Tofu.Ecs.Core.h"

namespace tofu
{
	struct RigidBody {
		b2Body* _body;
	};

	class Physics {
	public:
		Physics(observer_ptr<entt::registry> registry);

		// Transform��box2d���E�����킹��
		void FollowTransform();

		void Step(float time_step);

		// box2d���E��Transform�����킹��
		void WriteBackToTransform();

		RigidBody& GenerateBody(entt::entity entity);
		RigidBody& GenerateBody(entt::entity entity, const b2BodyDef& body_def);

	private:
		observer_ptr<entt::registry> _registry;
		std::unique_ptr<b2World> _world;
	};

}