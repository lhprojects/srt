#pragma once

#include "../Vec3.h"
#include "../Pars.h"
#include "../Source.h"
#include "DirectionSampler.h"
#include "PositionSampler.h"
#include "SourcesPars.h"
#include "Stop.h"

namespace srt {

	struct SphQuad {
		Vec3 o, x, y, z; // local reference system ¡¯R¡¯
		Real x0, y0, z0; // rectangle coords in ¡¯R¡¯
		Real x1, y1; //
		Real b0, b1; // misc precomputed constants
		Real Q0;
		Real S; // solid angle of ¡¯Q¡¯
		Real halfOmega;
	};

	struct PlaneStop : Stop
	{

		using Pars = pars::Pars<pars::origin_, pars::n1_, pars::n1Min_, pars::n1Max_,
			pars::n2_, pars::n2Min_, pars::n2Max_, pars::bound_>;

		PlaneStop(pars::uncheck_t, pars::argument auto const & ... args)
		{
			set(pars::uncheck, args...);
		}

		PlaneStop(pars::argument auto const & ... args)
		{
			pars::check<Pars>(args...);
			set(pars::uncheck, args...);
		}

		template<pars::argument... Args >
		void set(Args const &...args)
		{
			pars::check<Pars>(args...);
			set(pars::uncheck, args...);
		}

		template<pars::argument... Args >
		void set(pars::uncheck_t, Args const &...args)
		{
			pars::set(fO, pars::origin, args...);
			pars::set(fN1, pars::n1, args...);
			pars::set(fN2, pars::n2, args...);
			fN1 = normalize(fN1);
			fN2 = normalize(fN2);
			pars::set(fN1Min, pars::n1Min, args...);
			pars::set(fN1Max, pars::n1Max, args...);
			pars::set(fN2Min, pars::n2Min, args...);
			pars::set(fN2Max, pars::n2Max, args...);
			if constexpr (pars::has<Args...>(pars::bound)) {
				fBound_ = pars::get(pars::bound, args...);
				fBound = fBound_.get();
			}
		}

		// dProb = dA
		Vec3 sample() override;
		// dProb = cos(theta)/distance^2 dA
		Vec3 sample(Vec3 const& ref) override;
		bool accept(Vec3 const& ref,
			Vec3 const& d) override;
		Real solidAngle(Vec3 const& ref) override;


		Real fN1Min;
		Real fN1Max;
		Real fN2Min;
		Real fN2Max;
		Vec3 fO;
		Vec3 fN1;
		Vec3 fN2;
		Bound* fBound = nullptr;
		std::shared_ptr<Bound> fBound_ = nullptr;
	private:
		SphQuad fSQ;
	};

	inline std::shared_ptr<PlaneStop> planeStop(pars::argument auto const &... args)
	{
		pars::check<PlaneStop::Pars>(args...);
		return std::make_shared<PlaneStop>(args...);
	}

	struct StopDirectionSampler : DirectionSampler
	{
		Vec3 randomDirection(Vec3 const& o,
			Vec3 const& norm) override;

		bool accept(Vec3 const& o,
			Vec3 const& d,
			Vec3 const& norm) override;

		StopDirectionSampler(std::shared_ptr<Stop> stop,
			std::shared_ptr<DirectionSampler> ds) :
			fStop(std::move(stop)), fDS(std::move(fDS))
		{
		}
		std::shared_ptr<Stop> fStop;
		std::shared_ptr<DirectionSampler> fDS;
	};

	inline std::shared_ptr<StopDirectionSampler> stopDirectionSampler(std::shared_ptr<Stop> stop,
		std::shared_ptr<DirectionSampler> ds)
	{
		return std::make_shared<StopDirectionSampler>(std::move(stop), std::move(ds));
	}


	std::shared_ptr<Source> comSource(Real amp,
		std::shared_ptr<Spectrum> spectrum,
		std::shared_ptr<PositionSampler> ps,
		std::shared_ptr<DirectionSampler> ds);

	inline std::shared_ptr<Source> comSource(pars::uncheck_t,
		pars::argument auto const &... args)
	{
		return comSource(pars::get(pars::amp, args...),
			pars::get(pars::spectrum, args...),
			pars::get(pars::positionSampler, args...),
			pars::get(pars::directionSampler, args...));
	}

	inline std::shared_ptr<Source> comSource(
		pars::argument auto const &... args)
	{
		using Pars = pars::Pars<pars::amp_,
			pars::spectrum_,
			pars::directionSampler_,
			pars::positionSampler_>;

		pars::check<Pars>(args...);
		static_assert(pars::has<decltype(args)...>(pars::amp));
		static_assert(pars::has<decltype(args)...>(pars::spectrum));
		static_assert(pars::has<decltype(args)...>(pars::directionSampler),
			"must set direction sampler");
		static_assert(pars::has<decltype(args)...>(pars::positionSampler),
			"must set position sampler");
		return comSource(pars::uncheck, args...);
	}

}

