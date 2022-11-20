
#include "CosineDirectionSampler.h"

namespace srt
{
	Vec3 CosineSampler::randomDirection(Vec3 const& o,
		Vec3 const& norm)
	{
		return randomDiffuseRay(norm, fHalfCoineAngle);
	}

	CosineSampler::CosineSampler(Real halfCoineAngle)
	{
		fHalfCoineAngle = halfCoineAngle;
	}

	void CosineSampler::setHalfCoineAngle(Real halfCoineAngle)
	{
		fHalfCoineAngle = halfCoineAngle;
	}

	bool CosineSampler::accept(
		Vec3 const& o,
		Vec3 const& norm,
		Vec3 const& d)
	{

		if (fDoubleSide) {
			Real costheta = CosAngle(d, norm);
			return uniform(0, 1) < fabs(costheta);
		}
		else {
			Real costheta = CosAngle(d, norm);
			if (costheta < 0)
				return false;
			if (costheta >= cos(fHalfCoineAngle)) {
				return uniform(0, 1) <= costheta;
			}
			else {
				return false;
			}
		}
	}
}
