#pragma once

#include <cmath>

#ifdef TOFU_ENABLE_SIV3D
#include <Siv3D.hpp>
#endif

#ifdef TOFU_ENABLE_BOX2D
#include <box2d/box2d.h>
#endif

namespace tofu {
	template<class T>
	concept scalar_type = std::is_scalar_v<T>;

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

		constexpr bool operator==(const tVec2& other) const noexcept
		{
			return _x == other._x && _y == other._y;
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
}

