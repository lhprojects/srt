#pragma once

#include "../Pars.h"
#include "PositionSampler.h"

namespace srt {

	struct PlanePosSampler : PositionSampler
	{
		using Pars = pars::Pars<pars::origin_, pars::n1_, pars::n1Min_, pars::n1Max_,
				pars::n2_, pars::n2Min_, pars::n2Max_, pars::bound_>;

		PlanePosSampler(pars::uncheck_t,
			pars::argument auto const &...args)
		{
			set(pars::uncheck, args...);
		}

		PlanePosSampler(pars::argument auto const &...args)
		{
			pars::check<Pars>(args...);
			set(pars::uncheck, args...);
		}

		void set(pars::argument auto const &...args)
		{
			pars::check<Pars>(args...);
			set(pars::uncheck, args...);
		}

		void set(pars::uncheck_t, pars::argument auto const &...args)
		{
			pars::set(fO, pars::origin, args...);
			pars::set(fN1, pars::n1, args...);
			pars::set(fN2, pars::n2, args...);
			fN1 = normalize(fN1);
			fN2 = normalize(fN2);
			pars::set(fN1B0, pars::n1Min, args...);
			pars::set(fN1B1, pars::n1Max, args...);
			pars::set(fN2B0, pars::n2Min, args...);
			pars::set(fN2B1, pars::n2Max, args...);

			if constexpr (pars::has<decltype(args)...>(pars::bound)) {
				setBound(pars::get(pars::bound, args...));
			}
		}

		Vec3 sample(Vec3& norm) override;
		void setOrigin(Vec3 const& o) { fO = o; }
		void setNorms(Vec3 n1, Vec3 n2);
		void setNorm1Bounds(Real b10, Real b11) { fN1B0 = b10; fN1B1 = b11; }
		void setNorm2Bounds(Real b20, Real b21) { fN2B0 = b20; fN2B1 = b21; }

		void setBound(std::shared_ptr<Bound> b)
		{
			fBound = b;
		}
		Bound* getBound() { return fBound.get(); }
	private:
		Vec3 fO;
		Vec3 fN1, fN2;
		Real fN1B0, fN1B1, fN2B0, fN2B1;
		std::shared_ptr<Bound> fBound;
	};

	inline void PlanePosSampler::setNorms(Vec3 n1, Vec3 n2)
	{
		fN1 = normalize(n1);
		fN2 = normalize(n2);
	}

	inline Vec3 PlanePosSampler::sample(Vec3& norm)
	{
		for (;;) {
			Real n1 = uniform(fN1B0, fN1B1);
			Real n2 = uniform(fN2B0, fN2B1);
			Vec3 v = fO + fN1 * n1 + fN2 * n2;

			if (inBound(getBound(), v)) {
				norm = normalize(cross(fN1, fN2));
				return v;
			}
		}
	}

	std::shared_ptr<PlanePosSampler> planePositionSampler(
		pars::argument auto const &... args...)
	{
		pars::check<PlanePosSampler::Pars>(args...);
		return std::make_shared<PlanePosSampler>(args...);

	}

}
