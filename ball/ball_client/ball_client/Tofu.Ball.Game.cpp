#include "Tofu.Ball.Game.h"

#include "Tofu.Ecs.Core.h"
#include "Tofu.Ecs.Physics.h"
#include "Tofu.Ecs.Box2DPrimitiveRenderer.h"

namespace tofu::ball {
	UpdateSystem::UpdateSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry)
		: _serviceLocator(service_locator)
		, _registry(registry)
	{
	}

	void UpdateSystem::Step()
	{
		_serviceLocator->Get<TickCounter>()->Step();

		_serviceLocator->Get<Physics>()->FollowTransform();
		_serviceLocator->Get<Physics>()->Step(Scene::DeltaTime());
		_serviceLocator->Get<Physics>()->WriteBackToTransform();
	}

	ActionSystem::ActionSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry)
		: _serviceLocator(service_locator)
		, _registry(registry)
	{
	}

	void ActionSystem::Step()
	{
		auto actions = _serviceLocator->Get<ActionQueue>()->Retrieve();
		for (auto& action : actions)
		{
			std::visit([this, entity = action._entity](const auto& v) { this->apply(entity, v); }, action._action);
		}
	}

	void ActionSystem::apply(entt::entity entity, const actions::Dash& action)
	{
		auto body = _registry->get<RigidBody>(entity)._body;
	}

	Game::Game()
		: _end(false)
	{
	}
	void Game::run()
	{
		initialize();
		while (!_end && System::Update()) {
			game_loop();
		}
	}
	void Game::initialize()
	{
		Scene::SetBackground(ColorF(0.8, 0.9, 1.0));

		_end = false;

		// === Core ===
		_serviceLocator.Register(std::make_unique<TickCounter>());

		// === Physics ===
		auto physics = _serviceLocator.Register(std::make_unique<Physics>(&_registry));

		// === Simulation ===
		_serviceLocator.Register(std::make_unique<UpdateSystem>(&_serviceLocator, &_registry));
		_serviceLocator.Register(std::make_unique<ActionQueue>());
		
		// === Renderer ===
		_serviceLocator.Register(std::make_unique<Box2DPrimitiveRenderSystem>(&_registry, 100));

		{
			// floor
			auto entity = _registry.create();
			_registry.emplace<Transform>(entity, Float2{ 4.f, 6.f }, 0.0f);

			auto body = physics->GenerateBody(entity)._body;
			b2PolygonShape shape;
			shape.SetAsBox(4.0f, 0.1f);
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.1f;

			body->CreateFixture(&fixture_def);
			_registry.emplace<Floor>(entity);

			_registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
		}
		{
			// Wall
			auto entity = _registry.create();
			_registry.emplace<Transform>(entity, Float2{ 0.f, 3.f }, 0.0f);

			auto body = physics->GenerateBody(entity)._body;
			b2PolygonShape shape;
			shape.SetAsBox(0.1f, 3.0f);
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.1f;

			body->CreateFixture(&fixture_def);
			_registry.emplace<Floor>(entity);

			_registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
		}
		{
			// Wall
			auto entity = _registry.create();
			_registry.emplace<Transform>(entity, Float2{ 8.f, 3.f }, 0.0f);

			auto body = physics->GenerateBody(entity)._body;
			b2PolygonShape shape;
			shape.SetAsBox(0.1f, 3.0f);
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.1f;

			body->CreateFixture(&fixture_def);
			_registry.emplace<Floor>(entity);

			_registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
		}

		{
			auto entity = _registry.create();
			_registry.emplace<Transform>(entity, Float2{ 3.f, 0.5f }, 0.0f);
			auto ball = _registry.emplace<Ball>(entity, 0.08f);

			b2BodyDef body_def;
			body_def.type = b2BodyType::b2_dynamicBody;

			auto body = physics->GenerateBody(entity, body_def)._body;
			b2CircleShape shape;
			shape.m_radius = ball._radius;
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.4f;

			body->CreateFixture(&fixture_def);

			_registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::Aqua, Palette::Black);
		}

		{
			// box
			auto entity = _registry.create();
			_registry.emplace<Transform>(entity, Float2{ 2.5f, 0.5f }, 1.0f);

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

			_registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::Aqua, Palette::Black);
		}
	}
	void Game::game_loop()
	{
		if (KeyEscape.down())
		{
			_end = true;
		}

		if (KeyEnter.down())
		{
			auto view = _registry.view<Transform, Ball>();
			for (auto&& [entity, transform, ball] : view.proxy()) {
				transform._pos._y = 1;
			}
		}
		_serviceLocator.Get<UpdateSystem>()->Step();
		_serviceLocator.Get<Box2DPrimitiveRenderSystem>()->Render();
	}
}