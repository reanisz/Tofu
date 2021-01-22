#include "tofu/ball/stage.h"

#include <tofu/containers.h>
#include <tofu/ecs/core.h>
#include <tofu/ecs/physics.h>

// TODO: これの依存はあとで切る
#include <tofu/ecs/box2d_primitive_renderer.h>

namespace tofu::ball
{
	std::tuple<entt::entity> GoalFrame::Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, tVec2 pos)
	{
		auto entity = registry->create();
		registry->emplace<Transform>(entity, pos, 0.0f);
		auto physics = service_locator->Get<Physics>();

		registry->emplace<GoalFrame>(entity);
		
		b2BodyDef body_def;
		body_def.type = b2BodyType::b2_kinematicBody;

		auto body = physics->GenerateBody(entity, body_def)._body;

		auto apply = [](auto& vec, const Mat3x2& mat) {
			for (b2Vec2& v : vec) {
				auto s = s3d::Float2{ v.x, v.y };
				s = mat.transform(s);
				v = b2Vec2{s.x, s.y};
			}
		};

		const float w = 0.5f;
		const float h = 0.05f;

		for (int i = 0; i < 2; i++) {
			b2PolygonShape shape;
			stack_vector<b2Vec2, 8> v =
			{
				{-w, -h},
				{+w, -h},
				{+w, +h},
				{-w, +h},
			};
			int k = (i == 0 ? 1 : -1);
			auto offset = tVec2{w * k, 0};
			auto mat =
				s3d::Mat3x2::Rotate(ToRadians(-60 * k))
				* s3d::Mat3x2::Translate(offset);
			apply(v, mat);
			shape.Set(v.data(), v.size());
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			body->CreateFixture(&fixture_def);
		}


		registry->emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);

		return { entity };
	}

	std::tuple<entt::entity, Goal&> Goal::Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, tVec2 pos)
	{
		auto entity = registry->create();
		registry->emplace<Transform>(entity, pos, 0.0f);
		auto physics = service_locator->Get<Physics>();

		auto goal = registry->emplace<Goal>(entity);
		
		float d = 0.5f;

		b2BodyDef body_def;
		body_def.type = b2BodyType::b2_kinematicBody;

		auto body = physics->GenerateBody(entity, body_def)._body;
		b2PolygonShape shape;
		shape.SetAsBox(d / 2, d / 2);
		b2FixtureDef fixture_def;
		fixture_def.shape = &shape;
		fixture_def.isSensor = true;

		body->CreateFixture(&fixture_def);

		registry->emplace<Box2DPrimitiveRenderer>(entity, Color{255, 255, 0, 64}, Color{ 0,0,0,0 });

		goal._frame = std::get<0>(GoalFrame::Generate(service_locator, registry, pos));

		return { entity, goal };
	}

	void generate_stage(observer_ptr<tofu::ServiceLocator> service_locator, observer_ptr<entt::registry> registry)
    {
		auto physics = service_locator->Get<Physics>();

		{
			// floor
			auto entity = registry->create();
			registry->emplace<Transform>(entity, Float2{ 4.f, 6.f }, 0.0f);

			auto body = physics->GenerateBody(entity)._body;
			b2PolygonShape shape;
			shape.SetAsBox(4.0f, 0.1f);
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.1f;

			body->CreateFixture(&fixture_def);
			registry->emplace<Floor>(entity);

			registry->emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
		}
		{
			// ceil
			auto entity = registry->create();
			registry->emplace<Transform>(entity, Float2{ 4.f, 0.f }, 0.0f);

			auto body = physics->GenerateBody(entity)._body;
			b2PolygonShape shape;
			shape.SetAsBox(4.0f, 0.1f);
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.1f;

			body->CreateFixture(&fixture_def);
			registry->emplace<Floor>(entity);

			registry->emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
		}
		{
			// Wall
			auto entity = registry->create();
			registry->emplace<Transform>(entity, Float2{ 0.f, 3.f }, 0.0f);

			auto body = physics->GenerateBody(entity)._body;
			b2PolygonShape shape;
			shape.SetAsBox(0.1f, 3.0f);
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.1f;

			body->CreateFixture(&fixture_def);
			registry->emplace<Floor>(entity);

			registry->emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
		}
		{
			// Wall
			auto entity = registry->create();
			registry->emplace<Transform>(entity, Float2{ 8.f, 3.f }, 0.0f);

			auto body = physics->GenerateBody(entity)._body;
			b2PolygonShape shape;
			shape.SetAsBox(0.1f, 3.0f);
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.1f;

			body->CreateFixture(&fixture_def);
			registry->emplace<Floor>(entity);

			registry->emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
		}
		{
			// Left-Slope
			auto entity = registry->create();
			float d = 0.4f;
			registry->emplace<Transform>(entity, Float2{ d, 6 - d }, s3d::ToRadians(-45.f));

			auto body = physics->GenerateBody(entity)._body;
			b2PolygonShape shape;
			shape.SetAsBox(0.1f, d + 0.1f);
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.1f;

			body->CreateFixture(&fixture_def);
			registry->emplace<Floor>(entity);

			registry->emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
		}
		{
			// Right-Slope
			auto entity = registry->create();
			float d = 0.4f;
			registry->emplace<Transform>(entity, Float2{ 8 - d, 6 - d }, s3d::ToRadians(45.f));

			auto body = physics->GenerateBody(entity)._body;
			b2PolygonShape shape;
			shape.SetAsBox(0.1f, d + 0.1f);
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.1f;

			body->CreateFixture(&fixture_def);
			registry->emplace<Floor>(entity);

			registry->emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
		}

		{
			// goal
			Goal::Generate(service_locator, registry, { 0.5f, 3.5f });
			Goal::Generate(service_locator, registry, { 7.5f, 3.5f });
		}

		{
			// box
			auto entity = registry->create();
			registry->emplace<Transform>(entity, Float2{ 2.5f, 0.5f }, 1.0f);

			b2BodyDef body_def;
			body_def.type = b2BodyType::b2_dynamicBody;

			auto body = physics->GenerateBody(entity, body_def)._body;
			b2PolygonShape shape;
			shape.SetAsBox(0.06f, 0.06f);
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.4f;

			body->CreateFixture(&fixture_def);

			registry->emplace<Box2DPrimitiveRenderer>(entity, Palette::Aqua, Palette::Black);
		}
    }

}