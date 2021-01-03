#pragma once

#include <array>
#include <utility>

namespace tofu {
	// スタック上にデータが置かれるvector
	template<class T, std::size_t max>
	class stack_vector
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

		constexpr stack_vector() noexcept
			: _size(0)
		{
		}

		constexpr stack_vector(std::initializer_list<T> init_list)
			: _size(0)
		{
			*this = init_list;
		}

		~stack_vector()
		{
			erase(begin(), end());
		}

		constexpr stack_vector& operator=(const stack_vector& x)
		{
			clear();
			for (auto& v : x)
				push_back(v);
			return *this;
		}
		constexpr stack_vector& operator=(std::initializer_list<T> init_list)
		{
			assert(init_list.size() <= max);
			clear();

			for (auto& value : init_list) 
			{
				push_back(value);
			}

			return *this;
		}

		constexpr iterator begin() noexcept { return data(); }
		constexpr iterator end() noexcept { return data() + _size; }
		constexpr const_iterator begin() const noexcept { return data(); }
		constexpr const_iterator end() const noexcept { return data() + _size; }
		constexpr const_iterator cbegin() const noexcept { return data(); }
		constexpr const_iterator cend() const noexcept { return data() + _size; }
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
			return data()[n];
		}

		constexpr const_reference& operator[](size_type n) const
		{
			assert(n < size());
			return data()[n];
		}

		constexpr reference at(size_type n) 
		{
			if (size() <= n) {
				throw std::out_of_range("stack_vector: out of range");
			}
			assert(n < size());
			return data()[n];
		}

		constexpr reference at(size_type n) const
		{
			if (size() <= n) {
				throw std::out_of_range("stack_vector: out of range");
			}
			return data()[n];
		}

		constexpr T* data() noexcept
		{
			return reinterpret_cast<T*>(_buffer);
		}

		constexpr const T* data() const noexcept
		{
			return reinterpret_cast<const T*>(_buffer);
		}

	public:

		template<class U = T>
		constexpr void push_back(U&& x)
			requires std::is_convertible_v<U, T>
		{
			assert(!full());
			std::construct_at(&(data()[_size++]), std::forward<U>(x));
		}

		template<class... Args>
		constexpr void emplace_back(Args&&... args)
		{
			assert(!full());
			push_back(T{ std::forward<Args>(args)... });
		}

		constexpr void pop_back() noexcept
		{
			assert(!empty());
			_size--;
			std::destroy_at(&(data()[_size]));
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
			*position = std::forward<T>(x);

            _size++;

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
			std::destroy_at(&(data()[_size]));
			return position;
		}

		constexpr iterator erase(const_iterator position) 
		{
			return erase(const_cast<iterator>(position));
		}


		constexpr iterator erase(iterator first, iterator last)
		{
			if (first == last)
				return first;
			
			assert(!empty());
			int d = std::distance(first, last);

			if (last == end()) {
				_size -= d;
				for (int i = 0; i < d; i++) {
					std::destroy_at(&(data()[i + _size]));
				}

				return end();
			}
			iterator lhs = first;
			iterator rhs = last;

			while(lhs != last && rhs != end()) {
				using std::swap;
				swap(*lhs, *rhs);
				lhs++;
				rhs++;
			}
			_size -= d;
			for (int i = 0; i < d; i++) {
				std::destroy_at(&(data()[i + _size]));
			}

			return first;
		}

		constexpr iterator erase(const_iterator first, const_iterator last) 
		{
			return erase(const_cast<iterator>(first), const_cast<iterator>(last));
		}

	private:
		alignas(T) std::byte _buffer[sizeof(T) * max];
		size_type _size;
	};
}

