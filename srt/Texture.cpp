#ifndef SRT_TEXTURE_H
#define SRT_TEXTURE_H

#include "Real.h"
#include "Vec3.h"
#include "Texture.h"
#include <memory>
#include <algorithm>
#include <cmath>

namespace srt {

	void TextureInterface::logPoly(Vec3 const& pos, Real* c)
	{
		// keep the logs finite where the ratio is zero
		auto lnr = [&](Real t) {
			return log(std::max<Real>(ratio(pos, kSpecCenter + kSpecHalfWidth * t), 1E-300));
		};
		Real m = lnr(0);
		// even part c0 + c2 t^2 + c4 t^4, odd part c1 t + c3 t^3
		Real e1 = 0.5 * (lnr(0.5) + lnr(-0.5)) - m;
		Real e2 = 0.5 * (lnr(1) + lnr(-1)) - m;
		Real o1 = 0.5 * (lnr(0.5) - lnr(-0.5));
		Real o2 = 0.5 * (lnr(1) - lnr(-1));
		c[0] = m;
		c[2] = (16 * e1 - e2) / 3;
		c[4] = e2 - c[2];
		c[1] = (8 * o1 - o2) / 3;
		c[3] = o2 - c[1];
	}

	struct GaussSpectrum : TextureInterface {

		GaussSpectrum(
			Real reflect,
			Real lambda,
			Real sigma = 25) :
			fReflect(reflect),
			fSigma(sigma),
			fLambda(lambda)
		{
		}

		Real ratio(Vec3 const& pos, Real lambda) override
		{
			return fReflect * exp(-0.5 * Sqr(lambda - fLambda) / Sqr(fSigma));
		}

		// exact: ln ratio is a parabola in lambda
		void logPoly(Vec3 const&, Real* c) override
		{
			Real d = kSpecCenter - fLambda;
			Real s2 = Sqr(fSigma);
			c[0] = log(fReflect) - 0.5 * d * d / s2;
			c[1] = -kSpecHalfWidth * d / s2;
			c[2] = -0.5 * Sqr(kSpecHalfWidth) / s2;
			c[3] = 0;
			c[4] = 0;
		}

		Real fReflect;
		Real fSigma;
		Real fLambda;
	};

	std::shared_ptr<TextureInterface> gaussSpectrum(Real reflect,
		Real lambda,
		Real sigma)
	{
		return std::make_shared<GaussSpectrum>(reflect, lambda, sigma);
	}

}

#endif
