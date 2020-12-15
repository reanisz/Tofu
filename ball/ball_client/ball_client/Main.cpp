#include <Siv3D.hpp>
#include <cassert>

#include <entt\entt.hpp>

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
		void Register(std::unique_ptr<T>&& ptr) 
		{
			_container[get_id<T>()] = std::move(ptr);
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
		Point _pos;
		Angle _angle;
	};

	struct Ball {
		float _radius;
	};

	class BallRenderer {
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
			for (int i = 0; i < 10; i++) {
				auto entity = _registry.create();
				_registry.emplace<Transform>(entity, Point{ i * 50, i * 50 }, 0.0f);
				_registry.emplace<Ball>(entity, 8.0f);
			}
			_serviceLocator.Register(std::make_unique<BallRenderer>(&_registry));
		}
		void game_loop() 
		{
			if (KeyEscape.down())
			{
				_end = true;
			}

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
	// 背景を水色にする
	Scene::SetBackground(ColorF(0.8, 0.9, 1.0));

	tofu::Game game;
	game.run();
}

