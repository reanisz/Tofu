#include <Siv3D.hpp>
#include <cassert>
#include <optional>

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
	template<class T, std::size_t max>
	class static_vector
	{
	public:
		using reference = T&;
		using const_reference = const T&;
		using iterator = T*;
		using const_iterator = const T*;
		using size_type = std::size_t;
		using defference_type = std::ptrdiff_t;
		using value_type = T;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static_vector()
			: _size(0)
		{
		}

		static_vector& operator=(const static_vector& x)
		{
			_size = x._size;
			_data = x._data;
			return *this;
		}
		static_vector& operator=(std::initializer_list<T> init_list)
		{
			assert(init_list.size() <= max);
			_size = std::min(max, init_list.size());

			// 最適化の余地あり
			for (auto& value : init_list) 
			{
				push_back(value);
			}

			return *this;
		}

		iterator begin() { return _data.get(); }
		iterator end() { return _data.get() + _size; }
		iterator begin() const { return _data.get(); }
		iterator end() const { return _data.get() + _size; }
		const_iterator cbegin() const { return _data.get(); }
		const_iterator cend() const { return _data.get() + _size; }
		reverse_iterator rbegin() { return reverse_iterator{ end() }; }
		reverse_iterator rend() { return reverse_iterator{ begin() }; }
		const_reverse_iterator rbegin() const { return reverse_iterator{ end() }; }
		const_reverse_iterator rend() const { return reverse_iterator{ begin() }; }
		const_reverse_iterator crbegin() const { return reverse_iterator{ cend() }; }
		const_reverse_iterator crend() const { return reverse_iterator{ cbegin() }; }

		size_type size() const { return _size; }
		size_type capacity() const { return max; }
		bool empty() const { return _size == 0; }
		bool full() const { return _size == max; }

		reference& operator[](size_type n)
		{
			assert(n < size());
			return _data[n];
		}

		const_reference& operator[](size_type n) const
		{
			assert(n < size());
			return _data[n];
		}

		reference at(size_type n) 
		{
			if (size() <= n) {
				throw std::out_of_range("static_vector: out of range");
			}
			assert(n < size());
			return _data[n];
		}

		reference at(size_type n) const
		{
			if (size() <= n) {
				throw std::out_of_range("static_vector: out of range");
			}
			return _data[n];
		}

		T* data() 
		{
			return _data.data();
		}

		const T* data() const
		{
			return _data.data();
		}

		void push_back(const T& x)
		{
			assert(!full());
			_data[_size++] = x;
		}

		template<class... Args>
		void emplace_back(Args&&... args)
		{
			assert(!full());
			push_back(T{ std::forward(args)... });
		}

		void pop_back()
		{
			assert(!empty());
			_size--;
		}

		void clear()
		{
			_size = 0;
		}

	private:
		template<class U>
		iterator insert_impl(iterator position, U&& x)
		{
			assert(!full());
			for (iterator it = end(); it != position; it--) 
			{
				std::swap(*it, *(it - 1));
			}
			*position = std::forward(x);

			return position;
		}
	public:
		iterator insert(iterator position, const T& x)
		{
			return insert_impl(position, x);
		}
		iterator insert(const_iterator position, const T& x)
		{
			return insert_impl(const_cast<iterator>(position), x);
		}
		iterator insert(iterator position, T&& x)
		{
			return insert_impl(position, std::move(x));
		}

		// == std::vectorには存在するけど実装しなかった関数(insert) ==
		// void insert(iterator position, std::size_type n, const T& x);
		// iterator insert(const_iterator position, std::size_type n, const T& x);
		// template<class InputIterator>
		// void insert(iterator position, InputIterator first, InputIterator last);
		// template<class InputIterator>
		// void insert(const_iterator position, InputIterator first, InputIterator last);
		// iterator insert(const_iterator position, std::initializer_list<T> il);

		iterator erase(iterator position)
		{
			assert(!empty());
			for (iterator it = position; it != end() - 1; it++) 
			{
				std::swap(*it, *(it + 1));
			}
			_size--;
			return position;
		}

		iterator erase(const_iterator position) 
		{
			return erase(const_cast<iterator>(position));
		}

		iterator erase(iterator first, iterator last)
		{
			assert(!empty());
			if (last == end()) {
				_size -= std::distance(first, last);
				return end();
			}
			iterator lhs = first;
			iterator rhs = last;

			while(lhs != last && rhs != end()) {
				std::swap(*lhs, *rhs);
				lhs++;
				rhs++;
			}
			_size -= std::distance(first, last);
			return first;
		}

		iterator erase(const_iterator first, const_iterator last) 
		{
			return erase(const_cast<iterator>(first), const_cast<iterator>(last));
		}

	private:
		std::array<T, max> _data;
		size_type _size;
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

	struct Box2DPrimitiveRenderer
	{
		Color _fillColor;
		Color _frameColor;
	};

	class Box2DPrimitiveRenderSystem
	{
	public:
		Box2DPrimitiveRenderSystem(observer_ptr<entt::registry> registry) 
			: _registry(registry)
		{
		}

		void Render() 
		{
			auto view = _registry->view<Transform, RigidBody, Box2DPrimitiveRenderer>();
			for (auto&& [entity, transform, rigid_body, config] : view.proxy()) {
				auto body = rigid_body._body;
				auto fixture = body->GetFixtureList();
				do{
					auto shape = fixture->GetShape();
					switch (shape->GetType()) 
					{
					case b2Shape::e_circle:
						RenderShape(transform, config, dynamic_cast<b2CircleShape*>(shape));
						break;
					case b2Shape::e_polygon:
						RenderShape(transform, config, dynamic_cast<b2PolygonShape*>(shape));
						break;
					case b2Shape::e_edge:
						RenderShape(transform, config, dynamic_cast<b2EdgeShape*>(shape));
						break;
					case b2Shape::e_chain:
						RenderShape(transform, config, dynamic_cast<b2ChainShape*>(shape));
						break;
					}
				} while (fixture = fixture->GetNext());
			}
		}

	private:
		void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2CircleShape* shape) 
		{
			Circle{ transform._pos + trans(Float2{shape->m_p.x, shape->m_p.y}, transform._angle), shape->m_radius }
				.draw(config._fillColor)
				.drawFrame(1, config._frameColor);
		}
		void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2PolygonShape* shape) 
		{
			static_vector<Vec2, 8> v;
			float angle = transform._angle;

			auto p_shape = dynamic_cast<b2PolygonShape*>(shape);
			for (int i = 0; i < p_shape->m_count; i++) {
				auto pos = p_shape->m_vertices[i];
				v.push_back(transform._pos + trans(Vec2{ pos.x, pos.y }, angle));
			}

			Polygon polygon{ v.data(), v.size() };
			polygon.draw(config._fillColor).drawFrame(1, config._frameColor);
		}
		void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2EdgeShape* shape) 
		{
			auto p1 = transform._pos + trans(Float2{ shape->m_vertex1.x, shape->m_vertex1.y }, transform._angle);
			auto p2 = transform._pos + trans(Float2{ shape->m_vertex2.x, shape->m_vertex2.y }, transform._angle);

			Line{ p1, p2 }.draw(config._frameColor);
		}
		void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2ChainShape* shape) 
		{
			b2EdgeShape edge;
			for (int i = 0; i < shape->GetChildCount(); i++) 
			{
				shape->GetChildEdge(&edge, i);
				RenderShape(transform, config, &edge);
			}
		}

		template<class TVec>
		TVec trans(const TVec& pos, float angle)
		{
			return Mat3x2::Rotate(angle).transform(pos);
		}

	private:
		observer_ptr<entt::registry> _registry;
	};


	struct Ball 
	{
		float _radius;
	};

	struct Box 
	{
		b2Fixture* _fixture;
	};

	struct Controllable {
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
			_serviceLocator.Register(std::make_unique<Box2DPrimitiveRenderSystem>(&_registry));

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

				_registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
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
				fixture_def.restitution = 0.4f;

				body->CreateFixture(&fixture_def);

				_registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::Aqua, Palette::Black);
			}

			{
				// box
				auto entity = _registry.create();
				_registry.emplace<Transform>(entity, Point{ 250, 50 }, 1.0f);

				b2BodyDef body_def;
				body_def.type = b2BodyType::b2_dynamicBody;

				auto body = physics->GenerateBody(entity, body_def)._body;
				b2PolygonShape shape;
				shape.SetAsBox(6, 6);
				b2FixtureDef fixture_def;
				fixture_def.shape = &shape;
				fixture_def.density = 1.0f;
				fixture_def.friction = 0.9f;
				fixture_def.restitution = 0.4f;

				body->CreateFixture(&fixture_def);

				_registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::Aqua, Palette::Black);
			}
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
			_serviceLocator.Get<Box2DPrimitiveRenderSystem>()->Render();
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

