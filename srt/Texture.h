#pragma once

#include <memory>
#include "Real.h"
#include "Vec3.h"
#include "wavelength.h"

namespace srt {

	struct TextureInterface {
		virtual Real ratio(Vec3 const& pos, Real lambda) = 0;

		// whether ratio() changes with lambda
		virtual bool dependsOnWavelength() const { return true; }

		// ln ratio(pos, lambda) as c0 + c1 t + ... + c4 t^4 (see kSpecCenter);
		// by default the quartic through ln ratio at t = -1, -1/2, 0, 1/2, 1
		virtual void logPoly(Vec3 const& pos, Real* c);
	};

	std::shared_ptr<TextureInterface> gaussSpectrum(Real reflect,
		Real lambda,
		Real sigma = 25);

	constexpr Real WaveLengthRed = 620;
	constexpr Real WaveLengthYellow = 560;
	constexpr Real WaveLengthGreen = 520;
	constexpr Real WaveLengthBlue = 460;

	struct Texture
	{
		enum class TextureType {
			Homogenous,
			Function,
			Grid,	// squares of side fW alternating between ratio 1 and 0.5
		};

		Texture(Real r) : fRatio(r)
		{
		}

		void setFunction(std::shared_ptr<TextureInterface> r)
		{
			fType = TextureType::Function;
			fTextureInterface = std::move(r);
			fDependsOnWavelength = fTextureInterface->dependsOnWavelength();
			fRatio = 0;
		}

		// a grid on the plane with normal n
		void setGrid(Vec3 const& n, Real w)
		{
			fType = TextureType::Grid;
			fTextureInterface = nullptr;
			fDependsOnWavelength = false;
			fN1 = getNorm(normalize(n));
			fN2 = cross(normalize(n), fN1);
			fW = w;
			fRatio = 0;
		}

		void setRatio(Real r)
		{
			fType = TextureType::Homogenous;
			fTextureInterface = nullptr;
			fDependsOnWavelength = false;
			fRatio = r;
		}


		Real ratio(Vec3 const& pos, Real lambda) const
		{
			if (fType == TextureType::Homogenous) {
				return fRatio;
			}
			else if (fType == TextureType::Grid) {
				int a1 = (int)floor(dot(pos, fN1) / fW);
				int a2 = (int)floor(dot(pos, fN2) / fW);
				bool dark = (a1 & 1) ^ (a2 & 1);
				return dark ? 0.5 : 1;
			}
			else if (fType == TextureType::Function) {
				return fTextureInterface->ratio(pos, lambda);
			}
			else {
				throw "";
			}
		}
		bool dependsOnWavelength() const
		{
			return fDependsOnWavelength;
		}

		// only for a texture that dependsOnWavelength()
		void logPoly(Vec3 const& pos, Real* c) const
		{
			fTextureInterface->logPoly(pos, c);
		}

	private:
		TextureType fType = TextureType::Homogenous;
		std::shared_ptr<TextureInterface> fTextureInterface;
		Real fRatio;
		// asked once in setFunction, so hits need no virtual call
		bool fDependsOnWavelength = false;
		// for Grid
		Vec3 fN1, fN2;
		Real fW = 1;
	};

}

