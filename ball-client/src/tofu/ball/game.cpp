#include <Siv3D.hpp>

#include "tofu/ball/game.h"

#include "tofu/ecs/core.h"
#include "tofu/ecs/physics.h"
#include "tofu/ecs/box2d_primitive_renderer.h"

#include "tofu/ball/input.h"
#include "tofu/ball/actions.h"
#include "tofu/ball/stage.h"
#include "tofu/ball/player.h"
#include "tofu/ball/player_controller.h"
#include "tofu/ball/frame_updater.h"
#include "tofu/ball/renderer_registerer.h"

namespace tofu::ball 
{
	Game::Game()
		: _end(false)
	{
	}
    void Game::initBaseSystems()
    {
		initSystems();
    }
    void Game::initEnitites()
    {
		initStage();
		initBall();
		initPlayers();
    }
	void Game::run()
	{
		_end = false;
		_serviceLocator.Get<UpdateSystem>()->Start();

		while (!_end && System::Update()) {
			game_loop();
		}
	}
    observer_ptr<entt::registry> Game::getRegistry()
    {
        return &_registry;
    }
    observer_ptr<ServiceLocator> Game::getServiceLocator()
    {
        return &_serviceLocator;
    }

	void Game::initSystems()
	{
		// === Core ===
		_serviceLocator.Register(std::make_unique<TickCounter>());

		// === Input ===
		_serviceLocator.Register(std::make_unique<InputSystem>());

		// === Physics ===
		auto physics = _serviceLocator.Register(std::make_unique<Physics>(&_registry));

		// === Simulation ===
		_serviceLocator.Register(std::make_unique<UpdateSystem>(&_serviceLocator, &_registry));
		_serviceLocator.Register(std::make_unique<ActionQueue>());
		_serviceLocator.Register(std::make_unique<ActionSystem>(&_serviceLocator, &_registry));
		_serviceLocator.Register(std::make_unique<PlayerController>(&_serviceLocator, &_registry));
		
		// === Renderer ===
		auto camera = _serviceLocator.Register(std::make_unique<s3d::Camera2D>());
		camera->setCenter({ 400,300 });
		_serviceLocator.Register(std::make_unique<S3DRenderSystem>());
		_serviceLocator.Register(std::make_unique<Box2DPrimitiveRenderSystem>(&_serviceLocator, &_registry, 100));
	}
	void Game::initStage()
	{
        generate_stage(&_serviceLocator, &_registry);
	}
	void Game::initPlayers()
	{
		auto physics = _serviceLocator.Get<Physics>();
		{
			// Player 
			Player::Generate(&_serviceLocator, &_registry, 0, {1.0f, 5.0f});
			Player::Generate(&_serviceLocator, &_registry, 1, {7.0f, 5.0f});
		}
	}
	void Game::initBall()
	{
		auto physics = _serviceLocator.Get<Physics>();
		{
			// ball
			auto entity = _registry.create();
			_registry.emplace<Transform>(entity, Float2{ 4.f, 0.5f }, 0.0f);
			auto ball = _registry.emplace<Ball>(entity, 0.28f);

			b2BodyDef body_def;
			body_def.type = b2BodyType::b2_dynamicBody;

			auto body = physics->GenerateBody(entity, body_def)._body;
			b2CircleShape shape;
			shape.m_radius = ball._radius;
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.density = 0.22f;
			fixture_def.friction = 0.9f;
			fixture_def.restitution = 0.85f;

			body->CreateFixture(&fixture_def);
		}
	}
	void Game::game_loop()
	{
		if (KeyEscape.down())
		{
			_end = true;
		}

		auto transform = _serviceLocator.Get<Camera2D>()->createTransformer();

		_serviceLocator.Get<InputSystem>()->Update();

		if (_serviceLocator.Get<S3DRenderSystem>()->HasData()) {
			_serviceLocator.Get<S3DRenderSystem>()->Render();
		}
	}
}