#pragma once
#include "Real.h"
#include "Vec3.h"

namespace srt
{
	// result is not normalized
	inline Vec3 reflectDirection(Vec3 const& N, Vec3 const& rayD)
	{
		Vec3 d = rayD - 2. * N * dot(N, rayD);
		return d;
	}
}

