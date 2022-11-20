#ifndef SRT_RAY_H
#define SRT_RAY_H

#include "Real.h"
#include "Vec3.h"
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
		Vec3 fP; // polarization normalized

		Real fAmp;
		Real fLambda;

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
