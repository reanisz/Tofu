#include "Tofu.Ecs.Physics.h"

#include <box2d/box2d.h>

namespace tofu
{
	Physics::Physics(observer_ptr<entt::registry> registry)
		: _registry(registry)
	{
		_world = std::make_unique<b2World>(b2Vec2{ 0, 9.8f });
	}
	void Physics::FollowTransform()
	{
		for (auto&& [entity, transform, rigidbody] : _registry->view<Transform, RigidBody>().proxy()) {
			auto body = rigidbody._body;
			auto& pos = transform._pos;
			auto& angle = transform._angle;

			auto body_pos = body->GetPosition();
			auto body_angle = body->GetAngle();

			auto new_pos = b2Vec2{ pos._x, pos._y };
			if (body_pos == new_pos && body_angle == angle)
				continue;

			body->SetTransform(new_pos, angle);
			body->SetAwake(true);
		}
	}
	void Physics::Step(float time_step)
	{
		_world->Step(time_step, 6, 2);
	}
	void Physics::WriteBackToTransform()
	{
		for (auto&& [entity, transform, rigidbody] : _registry->view<Transform, RigidBody>().proxy()) {
			auto body = rigidbody._body;
			auto pos = body->GetPosition();
			auto angle = body->GetAngle();

			transform._pos._x = pos.x;
			transform._pos._y = pos.y;
			transform._angle = angle;
		}
	}
	RigidBody& Physics::GenerateBody(entt::entity entity)
	{
		b2BodyDef body_def;
		body_def.type = b2BodyType::b2_staticBody;
		return GenerateBody(entity, body_def);
	}
	RigidBody& Physics::GenerateBody(entt::entity entity, const b2BodyDef& body_def)
	{
		auto body = _world->CreateBody(&body_def);
		body->GetUserData().pointer = static_cast<std::uintptr_t>(entity);

		return _registry->emplace<RigidBody>(entity, body);
	}
}
