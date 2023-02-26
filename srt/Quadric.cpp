#include "Quadric.h"
#include <format>

namespace srt {

	std::string to_string(SymMatrix3X3 const& q)
	{
		return std::format("[{:9} {:9} {:9}, {:9} {:9} {:9}, {:9} {:9} {:9}]", q.fM11, q.fM12, q.fM13,
			q.fM12, q.fM22, q.fM23,
			q.fM13, q.fM23, q.fM33);
	}

	void Quadric::shift(Vec3 r)
	{
		// <Qx,x> + <P,x> + R = 0
		// <Q(x-o),(x-o)> + <P,(x-o)> + R = 0
		// <Qx,x> + <-2Qo+P,x> + <Qo,o> + <P,-o> + R = 0
		fR += dot(dot(fQ, r), r) + dot(fP, -r);
		fP += -2 * dot(fQ, r);
	}


	void Quadric::setSphere(
		Vec3 origin,
		Real radius)
	{
		fQ.fM11 = 1;
		fQ.fM22 = 1;
		fQ.fM33 = 1;
		fQ.fM12 = 0;
		fQ.fM13 = 0;
		fQ.fM23 = 0;
		fP = -2 * origin;
		fR = norm2(origin) - radius * radius;
	}


	void Sphere::setSphere(
		Vec3 origin,
		Real radius) {
		fP = -2. * origin;
		fR = norm2(origin) - radius * radius;
	}

	bool Sphere::inner(Vec3 const& p) const {
		return dot(fP + p, p) + fR < 0.;
	}


	void Quadric::setConicSurface(Vec3 origin,
		Vec3 d,
		Real radius,
		Real K)
	{
		d = normalize(d);

		// (n1 . r)^2 + (n2 . r)^2 - (2*radius)(d . r) + (K+1)(d . r)^2 = 0
		// (n1 . r)^2 + (n2 . r)^2 + (d . r)^2 + K (d . r)^2 - (2*radius)(d . r) = 0
		// r^2 + K(d . r)^2 - (2*radius)(d . r) = 0

		fQ.fM11 = 1. + K * d.fX * d.fX;
		fQ.fM22 = 1. + K * d.fY * d.fY;
		fQ.fM33 = 1. + K * d.fZ * d.fZ;

		fQ.fM12 = K * d.fX * d.fY;
		fQ.fM13 = K * d.fX * d.fZ;
		fQ.fM23 = K * d.fY * d.fZ;

		fP = -2 * radius * d;

		fR = 0;

		shift(origin);
	}

	void Quadric::setParabola(Vec3 origin,
		Vec3 d,
		Real radius)
	{
		setConicSurface(origin, d, radius, -1.);
	}

	void Quadric::setCone(Vec3 origin,
		Vec3 d,
		Real botRadius,
		Real topH,
		Real topRadius) {

		// (n1 . r)^2 + (n2 . r)^2 - (botRadius + h*(r . d))^2 = 0
		// r^2 - (1+h)(r . d)^2 - 2*botRadius*h*d . r - botRadius^2 = 0
		// h = (topRadius-botRadius)/topH

		d = normalize(d);

		Real h = (topRadius - botRadius) / topH;
		Real onePlush2 = 1 + h*h;
		fQ.fM11 = (1 - onePlush2 * d.fX * d.fX);
		fQ.fM22 = (1 - onePlush2 * d.fY * d.fY);
		fQ.fM33 = (1 - onePlush2 * d.fZ * d.fZ);

		fQ.fM12 = (-onePlush2 * d.fX * d.fY);
		fQ.fM13 = (-onePlush2 * d.fX * d.fZ);
		fQ.fM23 = (-onePlush2 * d.fY * d.fZ);

		fP = -2 * botRadius * h * d;

		fR = -botRadius * botRadius;

		shift(origin);


	}

	void Quadric::setTube(Vec3 origin,
		Vec3 d,
		Real radius)
	{
		// (n1 . r)^2 + (n2 . r)^2 - radius*radius = 0
		// r^2 - (d . r)^2 - (2*radius)(d . r) = 0

		d = normalize(d);

		fQ.fM11 = (1 - d.fX * d.fX);
		fQ.fM22 = (1 - d.fY * d.fY);
		fQ.fM33 = (1 - d.fZ * d.fZ);

		fQ.fM12 = (-d.fX * d.fY);
		fQ.fM13 = (-d.fX * d.fZ);
		fQ.fM23 = (-d.fY * d.fZ);

		fP = { 0,0,0 };

		fR = -radius * radius;

		shift(origin);
	}

	void Quadric::setSpheroid(Vec3 origin,
		Vec3 d,
		Real a, Real b)
	{
		d = normalize(d);

		// (d . r)^2 b^2/a^2 + ((n1 . r)^2 + (n2 . r)^2) = b^2
		// (d . r)^2 (r2-1) + r^2 = b^2

		Real r2 = (b / a) * (b / a);
		fQ.fM11 = 1. + (r2 - 1) * d.fX * d.fX;
		fQ.fM22 = 1. + (r2 - 1) * d.fY * d.fY;
		fQ.fM33 = 1. + (r2 - 1) * d.fZ * d.fZ;

		fQ.fM12 = (d.fX * d.fY) * (r2 - 1);
		fQ.fM13 = (d.fX * d.fZ) * (r2 - 1);
		fQ.fM23 = (d.fY * d.fZ) * (r2 - 1);

		fP = { 0,0,0 };

		fR = -b * b;

		shift(origin);
	}

	bool Quadric::inner(Vec3 const& p) const
	{
		return dot(dot(fQ, p), p) + dot(fP, p) + fR < 0.;
	}
}
