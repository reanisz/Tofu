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
			clear();

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

	// 数値型の別名
	template<class TTag, class TNumeric = int>
	struct StrongNumeric {
		using value_type = TNumeric;

		constexpr StrongNumeric()
			: _value(0)
		{
		}

		constexpr StrongNumeric(value_type value)
			: _value(value)
		{
		}

		constexpr operator value_type() const 
		{
			return _value;
		}

#define tofu_def_assign_op(op) \
		StrongNumeric& operator op(const StrongNumeric& other) \
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
		StrongNumeric& operator op() { \
			_value op; \
			return *this; \
		} \
		StrongNumeric operator op(int) { \
			auto res = *this; \
			_value op; \
			return res; \
		} \

		tofu_def_increment_op(++)
		tofu_def_increment_op(--)

#undef tofu_def_increment_op

		StrongNumeric operator+() const
		{
			return *this;
		}

		StrongNumeric operator-() const
		{
			if constexpr (std::is_signed_v<value_type>)
			{
				return -_value;
			}
			else {
				static_assert(false_v<value_type>, "value_type is not signed");
			}
		}

	private:
		value_type _value;
	};

#define tofu_def_binary_op(op) \
	template<class TTag, class TNumeric> \
	StrongNumeric<TTag, TNumeric> operator op(const StrongNumeric<TTag, TNumeric>& lhs, const StrongNumeric<TTag, TNumeric>& rhs) \
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

#define tofu_def_compare_op(op) \
	template<class TTag, class TNumeric> \
	bool operator op(const StrongNumeric<TTag, TNumeric>& lhs, const StrongNumeric<TTag, TNumeric>& rhs) \
	{ \
		return lhs._value op rhs._value; \
	} \

	tofu_def_compare_op(<)
	tofu_def_compare_op(<=)
	tofu_def_compare_op(>)
	tofu_def_compare_op(>=)
	tofu_def_compare_op(!=)
	tofu_def_compare_op(==)

#undef tofu_def_compare_op

	struct tVec2 
	{
		using value_type = float;

		constexpr tVec2()
			: _x(0)
			, _y(0)
		{
		}

		constexpr tVec2(value_type x, value_type y)
			: _x(x)
			, _y(y)
		{
		}

#ifdef TOFU_ENABLE_SIV3D
		constexpr tVec2(const s3d::Float2& other)
			: _x(other.x)
			, _y(other.y)
		{
		}
		constexpr operator s3d::Float2() const 
		{
			return s3d::Vec2{_x, _y};
		}
		constexpr tVec2(const s3d::Vec2& other)
			: _x(other.x)
			, _y(other.y)
		{
		}
		constexpr operator s3d::Vec2() const 
		{
			return s3d::Vec2{_x, _y};
		}
#endif

#ifdef TOFU_ENABLE_BOX2D
		constexpr tVec2(const b2Vec2& other)
			: _x(other.x)
			, _y(other.y)
		{
		}
		operator b2Vec2() const 
		{
			return b2Vec2{_x, _y};
		}
#endif

		value_type _x;
		value_type _y;

		tVec2 operator+() const 
		{
			return *this;
		}
		tVec2 operator-() const 
		{
			return { -_x, -_y };
		}

		constexpr tVec2& operator+=(const tVec2& other) {
			_x += other._x;
			_y += other._y;
			return *this;
		}

		constexpr tVec2& operator-=(const tVec2& other) {
			_x -= other._x;
			_y -= other._y;
			return *this;
		}

		constexpr tVec2& operator*=(value_type factor) {
			_x *= factor;
			_y *= factor;
			return *this;
		}

		constexpr tVec2& operator/=(value_type factor) {
			_x /= factor;
			_y /= factor;
			return *this;
		}
	};

	inline constexpr tVec2 operator+(const tVec2& lhs, const tVec2& rhs) 
	{
		auto ret = lhs;
		ret += rhs;
		return ret;
	}

	inline constexpr tVec2 operator-(const tVec2& lhs, const tVec2& rhs) 
	{
		auto ret = lhs;
		ret -= rhs;
		return ret;
	}

	template<scalar_type TScalar>
	inline constexpr tVec2 operator*(const tVec2& lhs, TScalar rhs) 
	{
		auto ret = lhs;
		ret *= rhs;
		return ret;
	}
	template<scalar_type TScalar>
	inline constexpr tVec2 operator*(TScalar lhs, const tVec2& rhs) 
	{
		auto ret = rhs;
		ret *= lhs;
		return ret;
	}
	template<scalar_type TScalar>
	inline constexpr tVec2 operator/(const tVec2& lhs, TScalar rhs) 
	{
		auto ret = lhs;
		ret /= rhs;
		return ret;
	}
	template<scalar_type TScalar>
	inline constexpr tVec2 operator/(TScalar lhs, const tVec2& rhs) 
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