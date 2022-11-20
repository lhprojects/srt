#ifndef SRT_SPECTRUM_H
#define SRT_SPECTRUM_H

#include "Real.h"

namespace srt {

	struct Spectrum
	{
		// return the wavelength
		virtual Real sample() = 0;
		virtual Real pdf(Real wavelength) = 0;
		virtual ~Spectrum() {}
	};
}

#endif
