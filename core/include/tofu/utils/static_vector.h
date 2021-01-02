#pragma once

#include <array>

namespace tofu {
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
}

