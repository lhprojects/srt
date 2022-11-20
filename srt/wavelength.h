#ifndef SRT_WAVELENGTH_H
#define SRT_WAVELENGTH_H

#include <stdint.h>
#include <math.h>
#include <utility>
#include "Real.h"

namespace srt {
	constexpr Real LEN_MIN = 380;
	constexpr Real LEN_MAX = 780;

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
