#ifndef SRT_WAVELENGTH_H
#define SRT_WAVELENGTH_H

#include <stdint.h>
#include <math.h>
#include <utility>
#include "Real.h"

namespace srt {
	constexpr Real LEN_MIN = 380;
	constexpr Real LEN_MAX = 780;

	// a spectrum S(lambda) is written as exp(c0 + c1 t + ... + c4 t^4), where
	// t = (lambda - kSpecCenter) / kSpecHalfWidth runs over [-1, 1] on
	// [LEN_MIN, LEN_MAX]
	constexpr int kSpecTerms = 5;
	constexpr Real kSpecCenter = 0.5 * (LEN_MIN + LEN_MAX);
	constexpr Real kSpecHalfWidth = 0.5 * (LEN_MAX - LEN_MIN);

	inline Real specPoly(Real const* c, Real t)
	{
		return c[0] + t * (c[1] + t * (c[2] + t * (c[3] + t * c[4])));
	}

	void WaveLength2XYZ(double len,
		double& x,
		double& y,
		double& z);

	void XYZ2RGB(double x, double y, double z,
		double& r, double& g, double& b);

	void WaveLength2RGB(double len,
		double* pr,
		double* pg,
		double* pb);

}
#endif
