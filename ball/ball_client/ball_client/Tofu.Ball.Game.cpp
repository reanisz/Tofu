#include "Tofu.Ball.Game.h"

#include "Tofu.Ecs.Core.h"
#include "Tofu.Ecs.Physics.h"
#include "Tofu.Ecs.Box2DPrimitiveRenderer.h"

namespace tofu::ball {

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
			static_vector<b2Vec2, 8> v =
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

	std::tuple<entt::entity, Player&> Player::Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, tVec2 pos)
	{
		auto physics = service_locator->Get<Physics>();

		auto entity = registry->create();
		registry->emplace<Transform>(entity, pos, 0.0f);

		auto player = registry->emplace<Player>(entity);
		player._id = 0;

		b2BodyDef body_def;
		body_def.type = b2BodyType::b2_dynamicBody;

		auto body = physics->GenerateBody(entity, body_def)._body;
		b2CircleShape shape;
		shape.m_radius = 0.14f;
		b2FixtureDef fixture_def;
		fixture_def.shape = &shape;
		fixture_def.density = 1.0f;
		fixture_def.friction = 0.9f;
		fixture_def.restitution = 0.4f;

		body->CreateFixture(&fixture_def);

		registry->emplace<Box2DPrimitiveRenderer>(entity, Palette::Aqua, Palette::Black);

		return { entity, player };
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

	void ActionSystem::apply(entt::entity entity, const actions::Move& action)
	{
		auto body = _registry->get<RigidBody>(entity)._body;
		auto d = action._target - tVec2{ body->GetPosition() };
		d.Normalize();
		body->ApplyForceToCenter(d * 0.7f, true);
	}

	void ActionSystem::apply(entt::entity entity, const actions::Dash& action)
	{
		auto body = _registry->get<RigidBody>(entity)._body;
		auto d = action._target - tVec2{ body->GetPosition() };
		d.Normalize();
		body->SetLinearVelocity({ 0, 0 });
		body->ApplyForceToCenter(d * 80, true);
	}

	PlayerController::PlayerController(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry)
		: _serviceLocator(service_locator)
		, _registry(registry)
	{
	}

	void PlayerController::Step()
	{
		for (auto&& [entity, player] : _registry->view<Player>().proxy()) 
		{
			auto queue = _serviceLocator->Get<ActionQueue>();
			auto clock = _serviceLocator->Get<TickCounter>();
			auto tick = clock->GetCurrent() + GameTick{ 0 };
			auto scale = _serviceLocator->Get<Box2DPrimitiveRenderSystem>()->GetScale();
			auto target = (1 / scale) * Cursor::PosF();
			if (MouseL.down()) {
				queue->Enqueue({ entity, actions::Dash{ target }, tick });
			}
			if (MouseR.pressed()) {
				queue->Enqueue({ entity, actions::Move{ target }, tick });
			}
		}
	}

	UpdateSystem::UpdateSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry)
		: _serviceLocator(service_locator)
		, _registry(registry)
		, _thread(std::chrono::milliseconds{ 15 }, [this](ScheduledUpdateThread&) { this->Step(); })
	{
	}

	void UpdateSystem::Start()
	{
		_thread.Start();
	}

	void UpdateSystem::Step()
	{
		_serviceLocator->Get<TickCounter>()->Step();
		auto tick = _serviceLocator->Get<TickCounter>()->GetCurrent();
		_serviceLocator->Get<ActionQueue>()->SetCurrentTick(tick);

		_serviceLocator->Get<PlayerController>()->Step();
		_serviceLocator->Get<ActionSystem>()->Step();

		_serviceLocator->Get<Physics>()->FollowTransform();
		_serviceLocator->Get<Physics>()->Step(1.f / 60);
		_serviceLocator->Get<Physics>()->WriteBackToTransform();
		
		_serviceLocator->Get<S3DRenderSystem>()->StartWrite();
		_serviceLocator->Get<Box2DPrimitiveRenderSystem>()->Render();
		_serviceLocator->Get<S3DRenderSystem>()->EndWrite();
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

		initSystems();
		initStage();
		initBall();
		initPlayers();

		_serviceLocator.Get<UpdateSystem>()->Start();
	}

	void Game::initSystems()
	{
		// === Core ===
		_serviceLocator.Register(std::make_unique<TickCounter>());

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
		auto physics = _serviceLocator.Get<Physics>();

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
			// ceil
			auto entity = _registry.create();
			_registry.emplace<Transform>(entity, Float2{ 4.f, 0.f }, 0.0f);

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
			// Left-Slope
			auto entity = _registry.create();
			float d = 0.4f;
			_registry.emplace<Transform>(entity, Float2{ d, 6 - d }, s3d::ToRadians(-45.f));

			auto body = physics->GenerateBody(entity)._body;
			b2PolygonShape shape;
			shape.SetAsBox(0.1f, d + 0.1f);
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
			// Left-Slope
			auto entity = _registry.create();
			float d = 0.4f;
			_registry.emplace<Transform>(entity, Float2{ 8 - d, 6 - d }, s3d::ToRadians(45.f));

			auto body = physics->GenerateBody(entity)._body;
			b2PolygonShape shape;
			shape.SetAsBox(0.1f, d + 0.1f);
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
			// goal
			Goal::Generate(&_serviceLocator, &_registry, { 0.5f, 3.5f });
			Goal::Generate(&_serviceLocator, &_registry, { 7.5f, 3.5f });
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
	void Game::initPlayers()
	{
		auto physics = _serviceLocator.Get<Physics>();
		{
			// Player 
			Player::Generate(&_serviceLocator, &_registry, {1.0f, 5.0f});
			Player::Generate(&_serviceLocator, &_registry, {7.0f, 5.0f});
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

			_registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
		}
	}
	void Game::game_loop()
	{
		if (KeyEscape.down())
		{
			_end = true;
		}

		auto transform = _serviceLocator.Get<Camera2D>()->createTransformer();

		// _serviceLocator.Get<UpdateSystem>()->Step();

		if (_serviceLocator.Get<S3DRenderSystem>()->HasData()) {
			_serviceLocator.Get<S3DRenderSystem>()->Render();
		}
	}
}