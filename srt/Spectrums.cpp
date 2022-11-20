#include "Spectrums.h"
#include "Random.h"

namespace srt {

	void MonoSpectrum::setWaveLength(Real lambda)
	{
		fWavelength = lambda;
	}

	Real MonoSpectrum::pdf(Real wavelength)
	{
		return 0;
	}

	Real MonoSpectrum::sample()
	{
		return fWavelength;
	}

	std::shared_ptr<MonoSpectrum> monoSpectrum(Real l)
	{
		return std::make_shared<MonoSpectrum>(l);
	}



	PlankLaw::PlankLaw(Real T)
	{
		fT = T;
	}

	Real PlankLaw::pdf(Real wavelength)
	{
		const Real hc_kB = 14.387E6; // nm K
		const Real alpha = hc_kB / wavelength / fT;
		Real a = pow(LambdaPeak() / wavelength, 5);
		Real b = (exp(alpha) - 1);
		return a / b;
	}

	Real PlankLaw::sample()
	{
		Real x = sampleX();
		Real lambda = lambdaFromX(x, fT);
		return lambda;
	}

	Real PlankLaw::sampleX() {
		// x= hv/kT=hc_kB / (T lambda)
		// 15/pi^4 x3/(e^x - 1)
		// https://digital.library.unt.edu/ark:/67531/metadc681096/m2/1/high_res_d/420378.pdf
		// http://www-star.st-and.ac.uk/~kw25/teaching/mcrt/mcrt_l06.pdf
		// https://math.stackexchange.com/questions/2521181/cdf-of-planck-distribution

		Real xi0 = uniform(0, 1);
		Real s00 = (Sqr(Sqr(kPi)) / 90.);
		Real s0 = xi0 * s00;
		Real s = 0;
		int L = 0;
		for (int j = 0; s < s0 && j < 1E4;) {
			++j;
			L = j;
			s += 1. / Sqr(Sqr(Real(j)));
		}
		Real xi4 = uniform(0, 1) * uniform(0, 1) * uniform(0, 1) * uniform(0, 1);
		Real x = (-1. / L) * log(xi4);
		return x;
	}

	Real PlankLaw::lambdaFromX(Real x, Real T) {
		const Real hc_kB = 14.387770E6; // nm K
		return (hc_kB / x) / T;
	}

	Real PlankLaw::LambdaPeak()
	{
		const Real x = 4.96511;
		return lambdaFromX(x, fT);
	}

	std::shared_ptr<PlankLaw> plankSpectrum(Real t)
	{
		return std::make_shared<PlankLaw>(t);
	}
}
