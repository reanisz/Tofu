#pragma once

#include <cassert>
#include <array>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <stdexcept>

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