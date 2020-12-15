#include <Siv3D.hpp>
#include <cassert>

#include <entt\entt.hpp>
#include <box2d/box2d.h>

namespace tofu {
	// https://en.cppreference.com/w/cpp/experimental/observer_ptr の簡易的な実装
	// TODO: テストを書く
	template<class T>
	struct observer_ptr {
		using pointer = T*;
		using element_type = T;

		observer_ptr() noexcept
			: _ptr(nullptr)
		{
		}
		observer_ptr(T* ptr) noexcept
			: _ptr(ptr)
		{
		}
		observer_ptr(std::nullptr_t) noexcept
			: _ptr(nullptr)
		{
		}

		observer_ptr<T>& operator=(T* ptr) noexcept
		{
			_ptr = ptr;
			return *this;
		}

		observer_ptr<T>& operator=(std::nullptr_t) noexcept
		{
			_ptr = nullptr;
			return *this;
		}

		void reset(T* ptr) noexcept
		{
			_ptr = ptr;
		}

		void reset(std::nullptr_t = nullptr) noexcept 
		{
			_ptr = nullptr;
		}

		typename pointer get() const noexcept {
			return _ptr;
		}

		typename element_type& operator*() const {
			assert(_ptr != nullptr);
			return *_ptr;
		}

		typename pointer operator->() const noexcept {
			assert(_ptr != nullptr);
			return _ptr;
		}

	private:
		T* _ptr;
	};

	// TODO: テストを書く
	class ServiceLocator 
	{
	public:
		template<class T>
		observer_ptr<T> Register(std::unique_ptr<T>&& ptr) 
		{
			_container[get_id<T>()] = std::move(ptr);
			return Get<T>();
		}

		template<class T>
		observer_ptr<T> Get() const 
		{
			return reinterpret_cast<T*>(_container.at(get_id<T>()).get());
		}

	private:
		template<class T>
		std::type_index get_id() const 
		{
			return std::type_index{ typeid(std::decay_t<T>) };
		}

		std::unordered_map<std::type_index, std::shared_ptr<void>> _container;
	};

	using Angle = float;
	struct Transform {
		Float2 _pos;
		Angle _angle;
	};

	struct RigidBody {
		b2Body* _body;
	};

	class Physics {
	public:
		Physics(observer_ptr<entt::registry> registry)
			: _registry(registry)
		{
			_world = std::make_unique<b2World>(b2Vec2{0, 9.8f});
		}

		void FollowTransform() 
		{
			for (auto&& [entity, transform, rigidbody] : _registry->view<Transform, RigidBody>().proxy()) {
				auto body = rigidbody._body;
				auto& pos = transform._pos;
				auto& angle = transform._angle;

				body->SetTransform({ pos.x, pos.y }, angle);
			}
		}

		void Step(float time_step) 
		{
			_world->Step(time_step, 6, 2);
		}

		void WriteBackToTransform() 
		{
			for (auto&& [entity, transform, rigidbody] : _registry->view<Transform, RigidBody>().proxy()) {
				auto body = rigidbody._body;
				auto pos = body->GetPosition();
				auto angle = body->GetAngle();

				transform._pos.x = pos.x;
				transform._pos.y = pos.y;
				transform._angle = angle;
			}
		}

		RigidBody& GenerateBody(entt::entity entity) 
		{
			b2BodyDef body_def;
			body_def.type = b2BodyType::b2_staticBody;
			return GenerateBody(entity, body_def);
		}
		RigidBody& GenerateBody(entt::entity entity, b2BodyDef body_def) 
		{
			auto body = _world->CreateBody(&body_def);
			body->GetUserData().pointer = static_cast<std::uintptr_t>(entity);

			return _registry->emplace<RigidBody>(entity, body);
		}

	private:
		observer_ptr<entt::registry> _registry;
		std::unique_ptr<b2World> _world;
	};


	struct Ball 
	{
		float _radius;
	};

	class BallRenderer 
	{
	public:
		BallRenderer(observer_ptr<entt::registry> registry) 
			: _registry(registry)
		{
		}

		void Render() 
		{
			auto view = _registry->view<Transform, Ball>();
			for (auto&& [entity, transform, ball] : view.proxy()) {
				Circle circle{ transform._pos, ball._radius };
				circle.draw();
				circle.drawFrame(1, Palette::Black);
			}
		}

	private:
		observer_ptr<entt::registry> _registry;
	};

	struct Box 
	{
		b2Fixture* _fixture;
	};

	class BoxRenderer 
	{
	public:
		BoxRenderer(observer_ptr<entt::registry> registry) 
			: _registry(registry)
		{
		}

		void Render() 
		{
			auto view = _registry->view<Transform, Box>();
			for (auto&& [entity, transform, box] : view.proxy()) {
				auto shape = box._fixture->GetShape();
				if (shape->GetType() != b2Shape::e_polygon)
					continue;

				Array<Vec2> v;

				auto p_shape = dynamic_cast<b2PolygonShape*>(shape);
				for (int i = 0; i < p_shape->m_count; i++) {
					auto pos = p_shape->m_vertices[i];
					v.push_back({ pos.x + transform._pos.x, pos.y + transform._pos.y });
				}

				Polygon polygon{ v };
				polygon.draw().drawFrame(1, Palette::Black);
			}
		}

	private:
		observer_ptr<entt::registry> _registry;
	};

	class Game {
	public:
		Game()
			: _end(false) 
		{
		}

		void run() 
		{
			initialize();
			while (!_end && System::Update()) {
				game_loop();
			}
		}

	private:

		void initialize() 
		{
			_end = false;

			auto physics = _serviceLocator.Register(std::make_unique<Physics>(&_registry));

			{
				// floor
				auto entity = _registry.create();
				_registry.emplace<Transform>(entity, Point{ 300, 500 }, 0.0f);

				b2BodyDef body_def;
				body_def.type = b2BodyType::b2_kinematicBody;
				auto body = physics->GenerateBody(entity, body_def)._body;
				b2PolygonShape shape;
				shape.SetAsBox(300, 10);
				b2FixtureDef fixture_def;
				fixture_def.shape = &shape;
				fixture_def.density = 1.0f;
				fixture_def.friction = 0.9f;
				fixture_def.restitution = 0.1f;

				auto fixture = body->CreateFixture(&fixture_def);
				_registry.emplace<Box>(entity, fixture);
			}
				
			for (int i = 0; i < 10; i++) {
				auto entity = _registry.create();
				_registry.emplace<Transform>(entity, Point{ i * 50, i * 50 }, 0.0f);
				auto ball = _registry.emplace<Ball>(entity, 8.0f);

				b2BodyDef body_def;
				body_def.type = b2BodyType::b2_dynamicBody;

				auto body = physics->GenerateBody(entity, body_def)._body;
				b2CircleShape shape;
				shape.m_radius = ball._radius;
				b2FixtureDef fixture_def;
				fixture_def.shape = &shape;
				fixture_def.density = 1.0f;
				fixture_def.friction = 0.9f;
				fixture_def.restitution = 0.1f;

				body->CreateFixture(&fixture_def);
			}

			_serviceLocator.Register(std::make_unique<BoxRenderer>(&_registry));
			_serviceLocator.Register(std::make_unique<BallRenderer>(&_registry));
		}
		void game_loop() 
		{
			if (KeyEscape.down())
			{
				_end = true;
			}

			if (KeyEnter.down())
			{
				auto view = _registry.view<Transform, Ball>();
				for (auto&& [entity, transform, ball] : view.proxy()) {
					transform._pos.y = 100;
				}
			}
			_serviceLocator.Get<Physics>()->FollowTransform();
			_serviceLocator.Get<Physics>()->Step(1 / 60.0f);
			_serviceLocator.Get<Physics>()->WriteBackToTransform();
			_serviceLocator.Get<BoxRenderer>()->Render();
			_serviceLocator.Get<BallRenderer>()->Render();
		}

	private:
		entt::registry _registry;
		ServiceLocator _serviceLocator;
		bool _end;
	};

}

void Main()
{
	Scene::SetBackground(ColorF(0.8, 0.9, 1.0));

	tofu::Game game;
	game.run();
}

