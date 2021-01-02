#pragma once

#include <array>

namespace tofu {
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
		tofu_def_assign_op(%=)

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

}
