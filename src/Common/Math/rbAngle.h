#pragma once

#include "../rbTypes.h"

#include "rbUtils.h"

namespace rbmk::Math
{
	class Angle
	{
		public:
			Angle &operator+=(const Angle &other) noexcept
			{
				m_fpAngle += other.m_fpAngle;
				return *this;
			}
			Angle &operator-=(const Angle &other) noexcept
			{
				m_fpAngle -= other.m_fpAngle;
				return *this;
			}

			Angle operator+(const Angle &other) const noexcept
			{
				Angle result = *this;
				result += other;
				return result;
			}

			Angle operator-(const Angle &other) const noexcept
			{
				Angle result = *this;
				result -= other;
				return result;
			}

			Angle operator-() const noexcept
			{
				Angle result;
				result.m_fpAngle = -m_fpAngle;
				return result;
			}

			Angle operator*(Float_t scalar) const noexcept
			{
				Angle result;
				result.m_fpAngle = m_fpAngle * scalar;
				return result;
			}

			Angle operator/(Float_t scalar) const noexcept
			{
				Angle result;
				result.m_fpAngle = m_fpAngle / scalar;
				return result;
			}

			Float_t m_fpAngle;
	};

	class Radian
	{
		public:
			Radian() : m_fpRadian{ 0 } {};
			explicit Radian(Float_t fpRadian) : m_fpRadian(fpRadian) {};
			Radian(const Angle &angle) : m_fpRadian(angle.m_fpAngle * PI_TO_RADIANS) {};

			inline Float_t Cos() const noexcept
			{
				return Cosf(m_fpRadian);
			}

			inline Float_t Sin() const noexcept
			{
				return Sinf(m_fpRadian);
			}

			inline Float_t Tan() const noexcept
			{
				return Tanf(m_fpRadian);
			}

			Radian &operator+=(const Radian &other) noexcept
			{
				m_fpRadian += other.m_fpRadian;
				return *this;
			}
			Radian &operator-=(const Radian &other) noexcept
			{
				m_fpRadian -= other.m_fpRadian;
				return *this;
			}
			Radian operator+(const Radian &other) const noexcept
			{
				Radian result = *this;
				result += other;
				return result;
			}
			Radian operator-(const Radian &other) const noexcept
			{
				Radian result = *this;
				result -= other;
				return result;
			}
			Radian operator-() const noexcept
			{
				Radian result;
				result.m_fpRadian = -m_fpRadian;
				return result;
			}
			Radian operator*(Float_t scalar) const noexcept
			{
				Radian result;
				result.m_fpRadian = m_fpRadian * scalar;
				return result;
			}
			Radian operator/(Float_t scalar) const noexcept
			{
				Radian result;
				result.m_fpRadian = m_fpRadian / scalar;
				return result;
			}

			Float_t m_fpRadian;
	};
}