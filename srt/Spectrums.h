#pragma once

#include <memory>
#include "Real.h"
#include "Spectrum.h"
#include "Pars.h"

namespace srt {

	struct MonoSpectrum : Spectrum
	{
		MonoSpectrum(Real l) : fWavelength(l) {}
		void setWaveLength(Real l);
		Real pdf(Real wavelength) override;
		Real sample() override;
	private:
		Real fWavelength;
	};
	
	std::shared_ptr<MonoSpectrum> monoSpectrum(Real l);

	struct PlankLaw : Spectrum
	{
		PlankLaw(Real T);
		// pdf for lambda
		// dProb/dlambda \propto Blambda \propto Ephoton dNphoton/dlambda
		Real pdf(Real wavelength) override;
		// sample lambda
		Real sample() override;
		Real LambdaPeak();

		static Real lambdaFromX(Real x, Real T);
		// dProb/dx \propto Bv \propto Ephoton dNphoton/dx
		static Real sampleX();

	private:
		Real fT;
	};

	namespace pars {
		struct temperature_; constexpr par<temperature_, Real> temperature;
		struct lambdaMin_; constexpr par<lambdaMin_, Real> lambdaMin;
		struct lambdaMax_; constexpr par<lambdaMax_, Real> lambdaMax;
	}

	std::shared_ptr<PlankLaw> plankSpectrum(Real t);

	std::shared_ptr<PlankLaw> plankSpectrum(pars::argument auto const &... args)
	{
		using Pars = pars::Pars<pars::temperature_, pars::lambdaMin_, pars::lambdaMax_>;
		pars::check<Pars>(args...);
		static_assert(pars::has<decltype(args)...>(pars::temperature),
			"must set temperature");
		std::shared_ptr<PlankLaw> ps = plankSpectrum(pars::get(pars::temperature, args...));
		static_assert(!pars::has<decltype(args)...>(pars::lambdaMin), "not support");
		static_assert(!pars::has<decltype(args)...>(pars::lambdaMax), "not support");
		return ps;
	}




}

