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

	void QuadricSurface::process(Ray const& r,
		ProcessHandler& handler) const
	{
		Real s;
		Vec3 inter, N;
		if (!intersect(r, s, inter, N)) {
			return;
		}
		if (handler.fType == HandlerType::Distance) {
			static_cast<DistanceHandler&>(handler).distance(s, dot(N, r.fD) > 0);
		} else if (handler.fType == HandlerType::Tracing) {
			static_cast<TracingHandler&>(handler).hitSurface(inter,
				N, dot(N, r.fD) > 0, this, this);
		}
	}

	std::string to_string(QuadricSurface const& q) {
		return std::format("Q = {};\nP = {};\nR = {}\n", q.fQ, q.fP, q.fR);
	}

	void SphereSurface::process(Ray const& r,
		ProcessHandler& handler) const {
		// <Qx,x> + <P,x> + R = 0
		// <Q(O+Ds>,O+Ds> + <P,O> + <P,D>s + R = 0

		Vec3 D = r.fD;
		Vec3 O = r.fO;
		Real c =  dot(fP + O, O) + fR;
		Real b = dot(O + 0.5 * fP, D);

		// s^2 + 2 b s + c = 0
		// s =  -b +- sqrt(b* b - c)
		// s =  c / (-b -+ sqrt(b*b-c))

		Real Delta = b * b - c;
		if (Delta <= 0) {
			// no intersection
		} else {

			Real s;
			Vec3 inter;
			Real sqrtD = sqrt(Delta);
			Real s1 = (-b - sqrtD);
			Real s2 = (-b + sqrtD);
			// -b + sqrt(b*b-c) < smin
			// b*b - c < 2*smin*b+b*b
			// c > -2*b*smin
			if (s2 <= gSmin) {
				// surface is behind ray
				return;
			} else {


				if (s1 <= gSmin/* && s2 > 0*/) {
					inter = r.fO + r.fD * s2;
					if (!inBound(getBound(), inter)) {
						return;
					}
					s = s2;
				} else { /*s1 > 0*/
				 // two intersections

					inter = O + D * s1;

					// try the first one
					if (inBound(getBound(), inter)) {
						s = s1;
					} else {
						// try the second one
						inter = O + D * s2;
						if (inBound(getBound(), inter)) {
							s = s2;
						} else {
							return;
						}
					}
				}

				Vec3 N = normalize(fP + 2. * inter);

				if (handler.fType == HandlerType::Distance) {
					static_cast<DistanceHandler&>(handler).distance(s, dot(N, r.fD) > 0);
					return;
				}


				if (handler.fType == HandlerType::Tracing) {
					static_cast<TracingHandler&>(handler).hitSurface(inter,
						N, dot(N, r.fD) > 0, this, this);
					return;
				}
			}
		}
	}

	ShiftSurface::ShiftSurface(std::shared_ptr<Surface> sur, Vec3 s) {
		fOrigin = std::move(sur);
		fShift = s;
	}

	void ShiftSurface::process(Ray const& r, ProcessHandler& handler) const
	{
		Ray ray = r;
		ray.shift(-fShift);
		fOrigin->process(r, handler);
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
