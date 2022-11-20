#include <memory>
#include "PositionSampler.h"

namespace srt {

	struct PointPositionSampler : PositionSampler {

		PointPositionSampler(Vec3 const& o, Vec3 const& d)
		{
			if (norm2(d) == 0) {
				throw "even point source need normal vector";
			}
			fO = o;
			fD = normalize(d);
		}

		Vec3 sample(Vec3& norm) override
		{
			norm = fD;
			return fO;
		}

	private:
		Vec3 fD;
		Vec3 fO;
	};

	std::shared_ptr<PositionSampler> pointPositionSampler(Vec3 const& o,
		Vec3 const& d)
	{
		return std::make_shared<PointPositionSampler>(o, d);
	}

}
