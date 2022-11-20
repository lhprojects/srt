#pragma once
#include "../Real.h"
#include "../Random.h"
#include "DirectionSampler.h"
#include <memory>


namespace srt {




	bool UniformDirectionSampler::accept(Vec3 const& o,
		Vec3 const& norm,
		Vec3 const& d)
	{
		if (fHalfCoineAngle == kPi) {
			return true;
		}
		else if (fHalfCoineAngle == 0.5 * kPi
			|| fHalfCoineAngle == kPi / 2) {
			return dot(d, norm) > 0;
		}

		Real costheta = CosAngle(d, norm);
		if (costheta >= cos(fHalfCoineAngle)) {
			return true;
		}
		else {
			return false;
		}
	}

}
