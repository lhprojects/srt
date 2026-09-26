#include "Surfaces.h"

namespace srt {


	bool PlaneSurface::isInner(Vec3 const& p) const {
		return inner(p);
	}
	void PlaneSurface::setGridTexture(Real w)
	{
		fIn2OutReflect.setGrid(fP, w);
		fOut2InReflect.setGrid(fP, w);
	}

	std::string to_string(QuadricSurface const& q) {
		return std::format("Q = {};\nP = {};\nR = {}\n", q.fQ, q.fP, q.fR);
	}

	bool SphereSurface::intersect(Ray const& r, Real tMax, Hit& hit) const
	{
		// <x,x> + <P,x> + R = 0
		// <O+Ds,O+Ds> + <P,O> + <P,D>s + R = 0

		Vec3 D = r.fD;
		Vec3 O = r.fO;
		Real c =  dot(fP + O, O) + fR;
		Real b = dot(O + 0.5 * fP, D);

		// s^2 + 2 b s + c = 0
		// s =  -b +- sqrt(b* b - c)

		Real Delta = b * b - c;
		if (Delta <= 0) {
			// no intersection
			return false;
		}
		Real sqrtD = sqrt(Delta);
		Real s1 = (-b - sqrtD);
		Real s2 = (-b + sqrtD);
		if (s2 <= gSmin) {
			// surface is behind ray
			return false;
		}

		// the first hit ahead of the ray within the bound
		Real ss[2] = { s1, s2 };
		for (int k = s1 <= gSmin ? 1 : 0; k < 2; ++k) {
			Real s = ss[k];
			if (s >= tMax) {
				return false;
			}
			Vec3 inter = O + D * s;
			if (inBound(getBound(), inter)) {
				hit.t = s;
				hit.in2out = dot(fP + 2. * inter, D) > 0;
				return true;
			}
		}
		return false;
	}

	void SphereSurface::shade(Ray const& r, Hit const& hit,
		TracingHandler& out) const
	{
		Vec3 inter = r.fO + r.fD * hit.t;
		out.hitSurface(inter, normalize(fP + 2. * inter), hit.in2out, this, this);
	}

	ShiftSurface::ShiftSurface(std::shared_ptr<Surface> sur, Vec3 s) {
		fOrigin = std::move(sur);
		fShift = s;
	}

	// as before, the ray is passed on unshifted
	bool ShiftSurface::intersect(Ray const& r, Real tMax, Hit& hit) const
	{
		if (!fOrigin->intersect(r, tMax, hit)) {
			return false;
		}
		if (!hit.sub) {
			hit.sub = fOrigin.get();
		}
		return true;
	}

	void ShiftSurface::shade(Ray const& r, Hit const& hit,
		TracingHandler& out) const
	{
		fOrigin->shade(r, hit, out);
	}

	bool ShiftSurface::isInner(Vec3 const& p) const
	{
		return fOrigin->isInner(p - fShift);
	}

	void ShiftSurface::shift(Vec3 const& p) {
		fShift += p;
	}

	std::shared_ptr<ShiftSurface> shift(std::shared_ptr<Surface> sur, Vec3 const& s)
	{
		return std::make_shared<ShiftSurface>(sur, s);
	}


}
