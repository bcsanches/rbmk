#pragma once

#include "../rbTypes.h"

#include <math.h>

namespace rbmk::Math
{
	inline Float_t Sqrtf(Float_t x)
	{
		return sqrtf(x);
	}

	inline Float_t Sinf(Float_t x)
	{
		return sinf(x);
	}

	inline Float_t Cosf(Float_t x)
	{
		return cosf(x);
	}

	inline Float_t Tanf(Float_t x)
	{
		return tanf(x);
	}

	Float_t constexpr PI = 3.14159265358979323846f;
	Float_t constexpr PI_TO_RADIANS = PI / 180.0f;
}
