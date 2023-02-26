#ifndef SRT_VEC3_H
#define SRT_VEC3_H

#include <math.h>
#include <string>
#include "Real.h"

namespace srt {

	struct Vec3
	{
		Real fX;
		Real fY;
		Real fZ;
	};

	std::string to_string(Vec3 const& q);


	inline Vec3 cross(Vec3 const& l, Vec3 const& r)
	{
		return {
			l.fY * r.fZ - l.fZ * r.fY,
			l.fZ * r.fX - l.fX * r.fZ,
			l.fX * r.fY - l.fY * r.fX,
		};
	}

	inline Vec3 operator-(Vec3 v)
	{
		return { -v.fX, -v.fY, -v.fZ };
	}

	inline Vec3 operator*(Vec3 const& l, Real s)
	{
		return { l.fX * s, l.fY * s, l.fZ * s };
	}

	inline Vec3& operator*=(Vec3& l, Real s)
	{
		l.fX *= s;
		l.fY *= s;
		l.fZ *= s;
		return l;
	}

	inline Vec3 operator*(Real s, Vec3 const& l)
	{
		return l * s;
	}

	inline Vec3& operator/=(Vec3& l, Real s)
	{
		l.fX *= 1. / s;
		l.fY *= 1. / s;
		l.fZ *= 1. / s;
		return l;
	}

	inline Vec3 operator/(Vec3 const& l, Real s)
	{
		return l * (1 / s);
	}

	inline Vec3 operator+(Vec3 const& l, Vec3 const& r)
	{
		return { l.fX + r.fX,
			l.fY + r.fY,
			l.fZ + r.fZ };
	}

	inline Vec3& operator+=(Vec3& l, Vec3 const& r)
	{
		l.fX += r.fX;
		l.fY += r.fY;
		l.fZ += r.fZ;
		return l;
	}

	inline Vec3 operator-(Vec3 const& l, Vec3 const& r)
	{
		return { l.fX - r.fX,
			l.fY - r.fY,
			l.fZ - r.fZ };
	}

	inline Real dot(Vec3 l, Vec3 r)
	{
		return l.fX * r.fX + l.fY * r.fY + l.fZ * r.fZ;
	};

	inline Real norm2(Vec3 v)
	{
		return dot(v, v);
	}

	inline Vec3 normalize(Vec3 const& v)
	{
		return v * (1. / sqrt(norm2(v)));
	}

	inline Real CosAngle(Vec3 v1, Vec3 v2)
	{
		return dot(normalize(v1), normalize(v2));
	}

	// get a vec3 norm to fD
	inline Vec3 getNorm(Vec3 const& fD)
	{
		Vec3 n1;
		if (fabs(fD.fX) <= fabs(fD.fY)) {
			if (fabs(fD.fX) <= fabs(fD.fZ)) {
				n1 = Vec3{ 1,0,0 } - fD * fD.fX;
			}
			else {
				n1 = Vec3{ 0,0,1 } - fD * fD.fZ;
			}
		}
		else {
			if (fabs(fD.fY) <= fabs(fD.fZ)) {
				n1 = Vec3{ 0,1,0 } - fD * fD.fY;
			}
			else {
				n1 = Vec3{ 0,0,1 } - fD * fD.fZ;
			}
		}
		return normalize(n1);
	}

}

#endif

