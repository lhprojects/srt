#include <memory>
#include "../Vec3.h"
#include "../Random.h"
#include "DirectionSampler.h"

namespace srt {

	 struct CosineSampler : DirectionSampler {

		CosineSampler() = default;
		CosineSampler(Real halfCoineAngle);
		void setHalfCoineAngle(Real hca);

		Vec3 randomDirection(Vec3 const& o,
			Vec3 const& norm) override;
		bool accept(Vec3 const& o,
			Vec3 const& norm,
			Vec3 const& d) override;

	private:
		bool fDoubleSide = true;
		Real fHalfCoineAngle = kPi / 2;
	};

	inline std::shared_ptr<CosineSampler> cosineDirectionSampler(
		Real angle = 0.5 * kPi)
	{
		return std::make_shared<CosineSampler>(angle);
	}

}
