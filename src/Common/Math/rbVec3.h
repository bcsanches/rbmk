#pragma once

#include "../rbTypes.h"

#include "rbUtils.h"

namespace rbmk::Math
{
	struct Vec3
	{
		Float_t x, y, z;

		inline Vec3() noexcept;
		inline Vec3(Float_t x, Float_t y, Float_t z) noexcept;

		inline Vec3 operator+(const Vec3 &v) const noexcept { return { x + v.x,y + v.y,z + v.z }; }
		inline Vec3 operator-(const Vec3 &v) const noexcept { return { x - v.x,y - v.y,z - v.z }; }
		inline Vec3 operator*(Float_t s) const noexcept { return { x * s,y * s,z * s }; }

		inline void operator+=(const Vec3 &v) noexcept { x += v.x; y += v.y; z += v.z; }

		inline void Normalize() noexcept;
		inline void Zero() noexcept;
		
		inline Float_t Dot(const Vec3 &rhs) const noexcept;

		inline static Vec3 Normalize(Float_t x, Float_t y, Float_t z) noexcept;
		inline static Vec3 Cross(const Vec3 &a, const Vec3 &b) noexcept;
	};

	inline Vec3::Vec3() noexcept
	{
		//empty
	}

	inline Vec3::Vec3(Float_t x, Float_t y, Float_t z) noexcept :
		x{ x },
		y{ y },
		z{ z }
	{
		//empty
	}

	inline void Vec3::Zero() noexcept
	{
		x = y = z = 0.0f;
	}

	inline Float_t Vec3::Dot(const Vec3 &rhs) const noexcept
	{
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	void Vec3::Normalize() noexcept
	{
		auto len = 1.0f / Sqrtf(this->Dot(*this));

		this->x *= len;
		this->y *= len;
		this->z *= len;
	}

	inline Vec3 Vec3::Normalize(Float_t x, Float_t y, Float_t z) noexcept
	{
		Vec3 v{ x,y,z };
		v.Normalize();
		return v;
	}

	inline Vec3 Vec3::Cross(const Vec3 &a, const Vec3 &b) noexcept
	{
		return 
		{
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		};
	}
}

