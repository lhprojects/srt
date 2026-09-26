#ifndef SRT_RAY_H
#define SRT_RAY_H

#include "Real.h"
#include "Vec3.h"
#include "wavelength.h"
#include <stdint.h>


namespace srt {

	struct Ray
	{
		Ray() = default;
		Ray(Vec3 o, Vec3 d, Real amp, Ray const& r);
		Ray(Vec3 o, Vec3 d, Real amp, Real freq, Vec3 p, int64_t id);
		void shift(Vec3 const& s);

		Vec3 fO;
		Vec3 fD; // direction normalized
		// polarization, normalized; (0,0,0) means not drawn yet: rays from
		// diffuse, metal or scattering events get one only when a surface
		// needs it (see ReflectType::Optical in Engine.cpp)
		Vec3 fP;

		Real fAmp;
		// wavelength; 0 means the ray carries the spectrum
		// exp(fC[0] + fC[1] t + ... + fC[4] t^4) (see kSpecCenter in wavelength.h)
		// instead, and a wavelength is drawn only when a surface needs one
		Real fLambda;
		Real fC[kSpecTerms] = {};

		int64_t fID;
	};

	inline void Ray::shift(Vec3 const& s) {
		fO += s;
	}

	inline Ray::Ray(Vec3 o, Vec3 d, Real amp, Ray const& r)
	{
		fO = o;
		fD = d;
		fAmp = amp;
		fLambda = r.fLambda;
		for (int k = 0; k < kSpecTerms; ++k) fC[k] = r.fC[k];
		fID = r.fID;
		fP = r.fP;
	}

	inline Ray::Ray(Vec3 o, Vec3 d, Real amp,
		Real freq, Vec3 p, int64_t id)
	{
		fO = o;
		fD = d;
		fAmp = amp;
		fLambda = freq;
		fID = id;
		fP = p;
	}
}

#endif
