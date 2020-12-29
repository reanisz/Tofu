#pragma once

#include <cassert>
#include <array>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <stdexcept>

#ifdef TOFU_ENABLE_SIV3D
#include <Siv3D.hpp>
#endif

#ifdef TOFU_ENABLE_BOX2D
#include <box2d/box2d.h>
#endif

namespace tofu {
	// Tに依存させたいときのやつ
	template<class T>
	constexpr bool true_v = true;
	template<class T>
	constexpr bool false_v = false;

	template<class T>
	concept scalar_type = std::is_scalar_v<T>;

	// https://en.cppreference.com/w/cpp/experimental/observer_ptr の簡易的な実装
	// TODO: テストを書く
	template<class T>
	struct observer_ptr {
		using pointer = T*;
		using element_type = T;

		constexpr observer_ptr() noexcept
			: _ptr(nullptr)
		{
		}
		constexpr observer_ptr(T* ptr) noexcept
			: _ptr(ptr)
		{
		}
		constexpr observer_ptr(std::nullptr_t) noexcept
			: _ptr(nullptr)
		{
		}

		constexpr observer_ptr<T>& operator=(T* ptr) noexcept
		{
			_ptr = ptr;
			return *this;
		}

		constexpr observer_ptr<T>& operator=(std::nullptr_t) noexcept
		{
			_ptr = nullptr;
			return *this;
		}

		constexpr void reset(T* ptr) noexcept
		{
			_ptr = ptr;
		}

		constexpr void reset(std::nullptr_t = nullptr) noexcept 
		{
			_ptr = nullptr;
		}

		constexpr typename pointer get() const noexcept {
			return _ptr;
		}

		constexpr typename element_type& operator*() const {
			assert(_ptr != nullptr);
			return *_ptr;
		}

		constexpr typename pointer operator->() const noexcept {
			assert(_ptr != nullptr);
			return _ptr;
		}

	private:
		T* _ptr;
	};

	// TODO: テストを書く
	// TODO: コンストラクタ/デストラクタをpush時/remove時に呼べるように変える
	// スタック上にデータが置かれるvector
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

		constexpr static_vector() noexcept
			: _size(0)
		{
		}

		constexpr static_vector(std::initializer_list<T> init_list)
			: _size(0)
		{
			*this = init_list;
		}

		constexpr static_vector& operator=(const static_vector& x)
		{
			_size = x._size;
			_data = x._data;
			return *this;
		}
		constexpr static_vector& operator=(std::initializer_list<T> init_list)
		{
			assert(init_list.size() <= max);
			clear();

			for (auto& value : init_list) 
			{
				push_back(value);
			}

			return *this;
		}

		constexpr iterator begin() noexcept { return _data.data(); }
		constexpr iterator end() noexcept { return _data.data() + _size; }
		constexpr iterator begin() const noexcept { return _data.data(); }
		constexpr iterator end() const noexcept { return _data.data() + _size; }
		constexpr const_iterator cbegin() const noexcept { return _data.data(); }
		constexpr const_iterator cend() const noexcept { return _data.data() + _size; }
		constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{ end() }; }
		constexpr reverse_iterator rend() noexcept { return reverse_iterator{ begin() }; }
		constexpr const_reverse_iterator rbegin() const noexcept { return reverse_iterator{ end() }; }
		constexpr const_reverse_iterator rend() const noexcept { return reverse_iterator{ begin() }; }
		constexpr const_reverse_iterator crbegin() const noexcept { return reverse_iterator{ cend() }; }
		constexpr const_reverse_iterator crend() const noexcept { return reverse_iterator{ cbegin() }; }

		constexpr size_type size() const noexcept { return _size; }
		constexpr size_type capacity() const noexcept { return max; }
		constexpr bool empty() const noexcept { return _size == 0; }
		constexpr bool full() const noexcept { return _size == max; }

		constexpr reference& operator[](size_type n)
		{
			assert(n < size());
			return _data[n];
		}

		constexpr const_reference& operator[](size_type n) const
		{
			assert(n < size());
			return _data[n];
		}

		constexpr reference at(size_type n) 
		{
			if (size() <= n) {
				throw std::out_of_range("static_vector: out of range");
			}
			assert(n < size());
			return _data[n];
		}

		constexpr reference at(size_type n) const
		{
			if (size() <= n) {
				throw std::out_of_range("static_vector: out of range");
			}
			return _data[n];
		}

		constexpr T* data() noexcept
		{
			return _data.data();
		}

		constexpr const T* data() const noexcept
		{
			return _data.data();
		}

		constexpr void push_back(const T& x)
		{
			assert(!full());
			_data[_size++] = x;
		}

		template<class... Args>
		constexpr void emplace_back(Args&&... args)
		{
			assert(!full());
			push_back(T{ std::forward(args)... });
		}

		constexpr void pop_back() noexcept
		{
			assert(!empty());
			_size--;
		}

		constexpr void clear() noexcept
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
		constexpr iterator insert(iterator position, const T& x)
		{
			return insert_impl(position, x);
		}
		constexpr iterator insert(const_iterator position, const T& x)
		{
			return insert_impl(const_cast<iterator>(position), x);
		}
		constexpr iterator insert(iterator position, T&& x)
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

		constexpr iterator erase(iterator position)
		{
			assert(!empty());
			for (iterator it = position; it != end() - 1; it++) 
			{
				std::swap(*it, *(it + 1));
			}
			_size--;
			return position;
		}

		constexpr iterator erase(const_iterator position) 
		{
			return erase(const_cast<iterator>(position));
		}

		constexpr iterator erase(iterator first, iterator last)
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

		constexpr iterator erase(const_iterator first, const_iterator last) 
		{
			return erase(const_cast<iterator>(first), const_cast<iterator>(last));
		}

	private:
		std::array<T, max> _data;
		size_type _size;
	};

	// 数値型の別名
	template<class TTag, class TNumeric = int>
	requires std::is_integral_v<TNumeric> || std::is_floating_point_v<TNumeric>
	struct StrongNumeric {
		using value_type = TNumeric;

		constexpr StrongNumeric() noexcept
			: _value(0)
		{
		}

		constexpr StrongNumeric(value_type value) noexcept
			: _value(value)
		{
		}

		constexpr explicit operator value_type() const noexcept
		{
			return _value;
		}

		constexpr value_type data() const noexcept
		{
			return _value;
		}

		constexpr value_type operator*() const noexcept
		{
			return _value;
		}

#define tofu_def_assign_op(op) \
		constexpr StrongNumeric& operator op(const StrongNumeric& other) noexcept \
		{ \
			_value op other._value; \
			return *this; \
		} \

		tofu_def_assign_op(+=)
		tofu_def_assign_op(-=)
		tofu_def_assign_op(*=)
		tofu_def_assign_op(/=)

#undef tofu_def_assign_op

#define tofu_def_increment_op(op) \
		constexpr StrongNumeric& operator op() noexcept { \
			_value op; \
			return *this; \
		} \
		constexpr StrongNumeric operator op(int) noexcept { \
			auto res = *this; \
			_value op; \
			return res; \
		} \

		tofu_def_increment_op(++)
		tofu_def_increment_op(--)

#undef tofu_def_increment_op

		constexpr StrongNumeric operator+() const noexcept
		{
			return *this;
		}

		constexpr StrongNumeric operator-() const noexcept
			requires std::is_signed_v<value_type>
		{
			return -_value;
		}

		constexpr auto operator<=>(const StrongNumeric&) const = default;

	private:
		value_type _value;
	};

#define tofu_def_binary_op(op) \
	template<class TTag, class TNumeric> \
	constexpr StrongNumeric<TTag, TNumeric> operator op(const StrongNumeric<TTag, TNumeric>& lhs, const StrongNumeric<TTag, TNumeric>& rhs) noexcept \
	{ \
		StrongNumeric<TTag, TNumeric> res = lhs; \
		res op ## = rhs; \
		return res; \
	}

	tofu_def_binary_op(+)
	tofu_def_binary_op(-)
	tofu_def_binary_op(*)
	tofu_def_binary_op(/)
	tofu_def_binary_op(%)
	// tofu_def_binary_op(<<)
	// tofu_def_binary_op(>>)
	// tofu_def_binary_op(^)
	// tofu_def_binary_op(&)
	// tofu_def_binary_op(|)

#undef tofu_def_binary_op

	struct tVec2 
	{
		using value_type = float;

		constexpr tVec2() noexcept
			: _x(0)
			, _y(0)
		{
		}

		constexpr tVec2(value_type x, value_type y) noexcept
			: _x(x)
			, _y(y)
		{
		}

#ifdef TOFU_ENABLE_SIV3D
		constexpr tVec2(const s3d::Float2& other) noexcept
			: _x(other.x)
			, _y(other.y)
		{
		}
		constexpr operator s3d::Float2() const  noexcept
		{
			return s3d::Vec2{_x, _y};
		}
		constexpr tVec2(const s3d::Vec2& other) noexcept
			: _x(static_cast<float>(other.x))
			, _y(static_cast<float>(other.y))
		{
		}
		constexpr operator s3d::Vec2() const noexcept
		{
			return s3d::Vec2{_x, _y};
		}
#endif

#ifdef TOFU_ENABLE_BOX2D
		constexpr tVec2(const b2Vec2& other) noexcept
			: _x(other.x)
			, _y(other.y)
		{
		}
		operator b2Vec2() const noexcept
		{
			return b2Vec2{_x, _y};
		}
#endif

		value_type _x;
		value_type _y;

		constexpr tVec2 operator+() const noexcept
		{
			return *this;
		}
		constexpr tVec2 operator-() const noexcept
		{
			return { -_x, -_y };
		}

		constexpr tVec2& operator+=(const tVec2& other) noexcept
		{
			_x += other._x;
			_y += other._y;
			return *this;
		}

		constexpr tVec2& operator-=(const tVec2& other) noexcept
		{
			_x -= other._x;
			_y -= other._y;
			return *this;
		}

		constexpr tVec2& operator*=(value_type factor) noexcept
		{
			_x *= factor;
			_y *= factor;
			return *this;
		}

		constexpr tVec2& operator/=(value_type factor) noexcept
		{
			_x /= factor;
			_y /= factor;
			return *this;
		}

		constexpr value_type LengthSquared() const noexcept
		{
			return _x * _x + _y * _y;
		}

		value_type Length() const noexcept
		{
			return std::sqrt(LengthSquared());
		}

		void Normalize() noexcept
		{
			auto length = Length();
			_x /= length;
			_y /= length;
		}

		tVec2 Normalized() const noexcept
		{
			auto res = *this;
			res.Normalize();
			return res;
		}
	};

	inline constexpr tVec2 operator+(const tVec2& lhs, const tVec2& rhs) noexcept
	{
		auto ret = lhs;
		ret += rhs;
		return ret;
	}

	inline constexpr tVec2 operator-(const tVec2& lhs, const tVec2& rhs) noexcept
	{
		auto ret = lhs;
		ret -= rhs;
		return ret;
	}

	template<scalar_type TScalar>
	inline constexpr tVec2 operator*(const tVec2& lhs, TScalar rhs) noexcept
	{
		auto ret = lhs;
		ret *= rhs;
		return ret;
	}
	template<scalar_type TScalar>
	inline constexpr tVec2 operator*(TScalar lhs, const tVec2& rhs) noexcept 
	{
		auto ret = rhs;
		ret *= lhs;
		return ret;
	}
	template<scalar_type TScalar>
	inline constexpr tVec2 operator/(const tVec2& lhs, TScalar rhs) noexcept
	{
		auto ret = lhs;
		ret /= rhs;
		return ret;
	}
	template<scalar_type TScalar>
	inline constexpr tVec2 operator/(TScalar lhs, const tVec2& rhs) noexcept 
	{
		auto ret = rhs;
		ret /= lhs;
		return ret;
	}

	// シンプルなサービスロケータ
	// TODO: テストを書く
	class ServiceLocator 
	{
	public:
		// 所有権を移譲してサービスを登録する
		template<class T>
		observer_ptr<T> Register(std::unique_ptr<T>&& ptr)
		{
			_container[get_id<T>()] = std::move(ptr);
			return Get<T>();
		}

		// サービスを取得
		template<class T>
		observer_ptr<T> Get() const
		{
			return reinterpret_cast<T*>(_container.at(get_id<T>()).get());
		}

		void clear() {
			_container.clear();
		}

	private:
		template<class T>
		std::type_index get_id() const
		{
			return std::type_index{ typeid(std::decay_t<T>) };
		}

		std::unordered_map<std::type_index, std::shared_ptr<void>> _container;
	};

}