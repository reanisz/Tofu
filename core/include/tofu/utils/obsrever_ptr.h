#pragma once

namespace tofu {
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

		constexpr operator bool() noexcept
		{
			return _ptr != nullptr;
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

		constexpr pointer get() const noexcept {
			return _ptr;
		}

		constexpr element_type& operator*() const {
			assert(_ptr != nullptr);
			return *_ptr;
		}

		constexpr pointer operator->() const noexcept {
			assert(_ptr != nullptr);
			return _ptr;
		}

	private:
		T* _ptr;
	};

}

