#include "Surfaces.h"

namespace srt {


	struct GridTexture : TextureInterface {

		GridTexture(Vec3 const& n, Real w)
		{
			n1 = getNorm(normalize(n));
			n2 = cross(normalize(n), n1);
			this->w = w;
		}
		Vec3 n1;
		Vec3 n2;
		Real w;
		Real ratio(Vec3 const& pos, Real lambda) override
		{
			Real x1 = dot(pos, n1);
			Real x2 = dot(pos, n2);
			x1 /= w;
			x2 /= w;
			int a1 = (int)floor(x1);
			int a2 = (int)floor(x2);
			bool dark = (a1 & 1) ^ (a2 & 1);
			if (dark) {
				return 0.5;
			}
			else {
				return 1;
			}
		}
	};

	bool PlaneSurface::isInner(Vec3 const& p) const {
		return inner(p);
	}
	void PlaneSurface::setGridTexture(Real w)
	{
		setReflect(std::make_shared<GridTexture>(fP, w));
	}

	void PlaneSurface::process(Ray const& r, ProcessHandler& handler) const
	{
		// <P,O+Ds> + R = 0
		Real b = dot(fP, r.fO) + fR;
		Real a = dot(fP, r.fD);
		// a s + b = 0

		if (a * b > 0) {
			return;
		}

		Real s1 = -b / a;

		if (s1 <= gSmin) {
			// surface is behind ray
			return;
		}
		else {
			Vec3 inter = r.fO + r.fD * s1;
			if (!inBound(getBound(), inter)) {
				return;
			}

			if (handler.fType == HandlerType::Distance) {
				static_cast<DistanceHandler&>(handler).distance(s1,
					b < 0);
			}
			else if (handler.fType == HandlerType::Tracing) {
				Vec3 N = normalize(fP);
				static_cast<TracingHandler&>(handler).hitSurface(inter,
					N,
					b < 0, this, this);
			}
		}

	}

	void QuadricSurface::process(Ray const& r,
		ProcessHandler& handler) const
	{
		// <Qx,x> + <P,x> + R = 0
		// <Q(O+Ds>,O+Ds> + <P,O> + <P,D>s + R = 0

		Vec3 D = r.fD;
		Vec3 O = r.fO;
		Vec3 QO = dot(fQ, O);
		Real c = dot(QO, O)
			+ dot(fP, O) + fR;
		Real b = dot(QO + 0.5 * fP, D);
		Real a = dot(dot(fQ, D), D);

		// a s^2 + 2 b s + c = 0
		// s =  (-b +- sqrt(b* b - a * c)) / a
		// s =  c / (-b -+ sqrt(b*b-a*c))

		Real Delta = b * b - a * c;
		if (Delta <= 0) {
			// no intersection
		}
		else {
			Real sqrtD = sqrt(Delta);
			Real alpha1 = (-b - sqrtD);
			Real alpha2 = (-b + sqrtD);

			Real s1, s2;

			if (fabs(a * c) < 0.1 * b * b) {
				if (b > 0) {
					if (a == 0)
						s1 = kInfity;
					else
						s1 = alpha1 / a;

					s2 = c / alpha1;
				}
				else {
					if (a == 0)
						s2 = kInfity;
					else
						s2 = alpha2 / a;
					s1 = c / alpha2;
				}
			}
			else {
				s1 = alpha1 / a;
				s2 = alpha2 / a;
			}
			if (s2 < s1) {
				std::swap(s1, s2);
			}

			if (s1 <= gSmin && s2 <= gSmin) {
				// surface is behind ray
				return;
			}
			else {

				Real s;
				Vec3 inter;

				if (s1 <= gSmin/* && s2 > 0*/) {
					inter = r.fO + r.fD * s2;
					if (!inBound(getBound(), inter)) {
						return;
					}
					s = s2;
				}
				else { /*s1 > 0*/
				 // two intersections

					inter = O + D * s1;

					// try the first one
					if (inBound(getBound(), inter)) {
						s = s1;
					}
					else {
						// try the second one
						inter = O + D * s2;
						if (inBound(getBound(), inter)) {
							s = s2;
						}
						else {
							return;
						}
					}
				}

				Vec3 N = normalize(fP + 2. * dot(fQ, inter));

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
