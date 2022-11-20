#include <memory>

#include "Stop.h"
#include "Sources.h"
#include "PositionSampler.h"
#include "DirectionSampler.h"

#include "../Source.h"
#include "../Real.h"
#include "../Spectrum.h"
#include "../Ray.h"
#include "../Random.h"
#include "../Bound.h"

namespace srt {


	struct ComSource : Source {

		ComSource(Real amp,
			std::shared_ptr<Spectrum> spectrum,
			std::shared_ptr<PositionSampler> ps,
			std::shared_ptr<DirectionSampler> ds) :
			Source(amp),
			fDirection(std::move(ds)),
			fPosition(std::move(ps)),
			fSpectrum(std::move(spectrum))
		{
		}

		Ray generate() override;

		std::shared_ptr<DirectionSampler> fDirection = nullptr;
		std::shared_ptr<PositionSampler> fPosition = nullptr;
		std::shared_ptr<Spectrum> fSpectrum;
	};

	Ray ComSource::generate()
	{
		Vec3 norm;
		Vec3 o = fPosition->sample(norm);
		Vec3 d = fDirection->randomDirection(o, norm);
		Real lambda = fSpectrum->sample();
		Ray ray;
		ray.fP = randomNorm(d);
		ray.fO = o;
		ray.fD = d;
		ray.fAmp = 1.;
		ray.fLambda = lambda;
		return ray;
	}

	std::shared_ptr<Source> comSource(Real amp,
		std::shared_ptr<Spectrum> spectrum,
		std::shared_ptr<PositionSampler> ps,
		std::shared_ptr<DirectionSampler> ds)
	{
		return std::make_shared<ComSource>(
			std::move(amp),
			std::move(spectrum),
			std::move(ps),
			std::move(ds));
	}




	Vec3 PlaneStop::sample()
	{
		for (;;) {
			Vec3 inter = uniform(fN1Min, fN1Max) * fN1 +
				uniform(fN2Min, fN2Max) * fN2 +
				fO;
			if (inBound(fBound, inter)) {
				return inter;
			}
		}
	}



	Real solid_angle_triangle(Vec3 R, Vec3 r1, Vec3 r2)
	{
		Real R0 = sqrt(norm2(R));
		Real R1 = sqrt(norm2(R + r1));
		Real R2 = sqrt(norm2(R + r2));
		Real R12 = dot(R + r1, R + r2);
		Real R01 = dot(R, R + r1);
		Real R02 = dot(R, R + r2);

		Real area = -dot(R, cross(r1, r2));
		Real E = 2 * atan(area / (R1 * R2 * R0 + R0 * R12
			+ R1 * R02 + R2 * R01));
		return E;
	}


	Real clamp(Real v, Real lo, Real hi)
	{
		return v < lo ? lo : hi < v ? hi : v;
	}


	Real solid(Real z0, Real x0, Real y0,
		Real w, Real h)
	{
		Vec3 r1{ w, 0, 0 };
		Vec3 r2{ 0, h, 0 };
		Vec3 Ra{ x0, y0, z0 };
		Vec3 Rb{ x0 + w, y0 + h, z0 };
		Real a = solid_angle_triangle(Ra, r1, r2);
		Real b = solid_angle_triangle(Rb, -r1, -r2);
		return a + b;
	}

	bool PlaneStop::accept(Vec3 const& ref,
		Vec3 const& d)
	{
		Vec3 n3 = cross(fN1, fN2);
		Real s0 = dot(fO, n3);
		Real sref = dot(ref, n3);
		Real sd = dot(d, n3);
		Real k = (s0 - sref) / sd;
		if (k <= 0) return false;
		Vec3 inter = ref + k * d;
		return inBound(fBound, inter);
	}

	void SphQuadInit(SphQuad& squad, Vec3 s,
		Vec3 ex, Vec3 ey, Vec3 o)
	{
		squad.o = o;
		Real exl = sqrt(norm2(ex)),
			eyl = sqrt(norm2(ey));
		// compute local reference system 'R'
		squad.x = ex / exl;
		squad.y = ey / eyl;
		squad.z = cross(squad.x, squad.y);
		// compute rectangle coords in local reference system
		Vec3 d = s - o;
		squad.z0 = dot(d, squad.z);
		// flip 'z' to make it point against 'Q'
		if (squad.z0 > 0) {
			squad.z *= -1;
			squad.z0 *= -1;
		}
		squad.x0 = dot(d, squad.x);
		squad.y0 = dot(d, squad.y);
		squad.x1 = squad.x0 + exl;
		squad.y1 = squad.y0 + eyl;


		squad.b0 = -squad.y0 / sqrt(Sqr(squad.z0) + Sqr(squad.y0));
		squad.b1 = squad.y1 / sqrt(Sqr(squad.z0) + Sqr(squad.y1));

		squad.Q0 = solid(squad.z0, 0, squad.y0,
			squad.x0, eyl);

		squad.S = solid(squad.z0, squad.x0, squad.y0,
			exl, eyl);

		if (squad.y0 * squad.y1 > 0)
			squad.halfOmega = atan(eyl / (-squad.z0 * (1 + squad.y0 * squad.y1 / Sqr(squad.z0))));
		else
			squad.halfOmega = atan(-squad.y1 / squad.z0) - atan(-squad.y0 / squad.z0);
	}


	Real PlaneStop::solidAngle(Vec3 const& ref)
	{
		Vec3 s = fO + fN1 * fN1Min + fN2 * fN2Min;
		SphQuadInit(fSQ, s, fN1 * (fN1Max - fN1Min),
			fN2 * (fN2Max - fN2Min), ref);
		return fSQ.S;
	}

	Vec3 SphQuadSample(SphQuad const& squad, Real u, Real v)
	{
		// 1. compute 'cu'
		// u(this function) = 1 - u(definiton in origin paper)
		Real Q = squad.Q0 + u * squad.S;
		Real a0 = sin(squad.halfOmega);
		Real a1 = sin(0.5 * (squad.halfOmega + Q));
		Real a2 = sin(0.5 * (squad.halfOmega - Q));
		Real cu = sin(Q) / sqrt(
			Sqr(a0) + 4 * squad.b0 * squad.b1 * a1 * a2);

		//cu = clamp(cu, -1, 1); // avoid NaNs
		// 2. compute 'xu'
		Real xu = -(cu * squad.z0) / sqrt(1 - cu * cu);
		//xu = clamp(xu, squad.x0, squad.x1); // avoid Infs

		// 3. compute 'yv'
		Real d = sqrt(xu * xu + Sqr(squad.z0));
		Real h0 = squad.y0 / sqrt(Sqr(d) + Sqr(squad.y0));
		Real h1 = squad.y1 / sqrt(Sqr(d) + Sqr(squad.y1));
		Real hv = h0 + v * (h1 - h0);
		Real hv2 = Sqr(hv);
		Real yv = (hv * d) / sqrt(1 - hv2);
		//yv = clamp(yv, squad.y0, squad.y1);

		// 4. transform (xu,yv,z0) to world coords
		return (squad.o + xu * squad.x + yv * squad.y +
			squad.z0 * squad.z);
	}

	Vec3 PlaneStop::sample(Vec3 const& ref)
	{
		Vec3 s = fO + fN1 * fN1Min + fN2 * fN2Min;
		SphQuadInit(fSQ, s, fN1 * (fN1Max - fN1Min),
			fN2 * (fN2Max - fN2Min), ref);

		for (;;) {

			// https://www.arnoldrenderer.com/research/egsr2013_spherical_rectangle.pdf
			Vec3 inter = SphQuadSample(fSQ,
				uniform(0, 1), uniform(0, 1));

			if (inBound(fBound, inter)) {
				return inter;
			}
		}
	}

	Vec3 StopDirectionSampler::randomDirection(Vec3 const& o,
		Vec3 const& norm)
	{
		for (;;) {
			Vec3 stopPoint = fStop->sample(o);
			Vec3 d = normalize(stopPoint - o);
			if (!fDS || fDS->accept(o, norm, d)) {
				return d;
			}
		}
	}

	bool StopDirectionSampler::accept(Vec3 const& o,
		Vec3 const& norm,
		Vec3 const& d)
	{
		return fStop->accept(o, d)
			&& (!fDS || fDS->accept(o, norm, d));
	}

}





