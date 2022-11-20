#pragma once
#include "../Vec3.h"
#include "../Real.h"
#include "../Random.h"
#include <memory>

namespace srt {

	struct DirectionSampler
	{
		// dProb ~ dL/dOmega (at o)
		virtual Vec3 randomDirection(Vec3 const& o,
			Vec3 const& norm) = 0;

		// dProb ~ dL/dOmega (at o)
		virtual bool accept(Vec3 const& o,
			Vec3 const& norm,
			Vec3 const& d) = 0;
	};

	struct UniformDirectionSampler : DirectionSampler
	{

		UniformDirectionSampler(Real halfCoineAngle = kPi)
			: fHalfCoineAngle(halfCoineAngle)
		{
		}

		void setHalfCoineAngle(Real hca) { fHalfCoineAngle = hca; }

		Vec3 randomDirection(Vec3 const& o, Vec3 const& norm) override
		{
			return randomUniformRay(norm, fHalfCoineAngle);
		}

		bool accept(Vec3 const& o,
			Vec3 const& norm,
			Vec3 const& d) override;

	private:
		Real fHalfCoineAngle;
	};

	inline std::shared_ptr<DirectionSampler>
		uniformDirectionSampler(Real halfCoineAngle = kPi)
	{
		return std::make_shared<UniformDirectionSampler>(halfCoineAngle);
	}

}
