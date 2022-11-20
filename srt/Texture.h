#pragma once

#include <memory>
#include "Real.h"
#include "Vec3.h"

namespace srt {

	struct TextureInterface {
		virtual Real ratio(Vec3 const& pos, Real lambda) = 0;
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
		};

		Texture(Real r) : fRatio(r)
		{
		}

		void setFunction(std::shared_ptr<TextureInterface> r)
		{
			fType = TextureType::Function;
			fTextureInterface = std::move(r);
			fRatio = 0;
		}

		void setRatio(Real r)
		{
			fType = TextureType::Homogenous;
			fTextureInterface = nullptr;
			fRatio = r;
		}


		Real ratio(Vec3 const& pos, Real lambda) const
		{
			if (fType == TextureType::Homogenous) {
				return fRatio;
			}
			else if (fType == TextureType::Function) {
				return fTextureInterface->ratio(pos, lambda);
			}
			else {
				throw "";
			}
		}
	private:
		TextureType fType = TextureType::Homogenous;
		std::shared_ptr<TextureInterface> fTextureInterface;
		Real fRatio;
	};

}

