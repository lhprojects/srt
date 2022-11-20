#ifndef SRT_TEXTURE_H
#define SRT_TEXTURE_H

#include "Real.h"
#include "Vec3.h"
#include "Texture.h"
#include <memory>

namespace srt {

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
