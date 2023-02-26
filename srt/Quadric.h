#ifndef SRT_QUADRIC_H
#define SRT_QUADRIC_H

#include "Vec3.h"
#include "Real.h"
#include "Pars.h"

namespace srt {

	struct SymMatrix3X3 {

		SymMatrix3X3();
		SymMatrix3X3(Real m1, Real m12, Real m13,
			Real m22, Real m23, Real m33);

		Real fM11{};
		Real fM12{};
		Real fM13{};
		Real fM22{};
		Real fM23{};
		Real fM33{};

		SymMatrix3X3 operator-() const;

	};
	Vec3 dot(SymMatrix3X3 const& q, Vec3 const& v);


	struct Sphere {

		static constexpr auto pars_ = pars::shape | pars::origin | pars::radius;

		void setSphere(
			Vec3 center,
			Real radius);
		bool inner(Vec3 const& p) const;

		void set(pars::argument auto const &... args) {
			pars::check(pars_, args...);
			set(pars::uncheck, args...);
		}

		void set(pars::uncheck_t, pars::argument auto const &... args) {
			constexpr bool a = pars::has<decltype(args)...>(pars::origin);
			constexpr bool b = pars::has<decltype(args)...>(pars::radius);

			static_assert((a==b), "set origin must set radius");
			if constexpr (a && b) {
				setSphere(pars::get(pars::origin, args...),
					pars::get(pars::radius, args...));
			}
		}

		Real fR;
		Vec3 fP;
	};

	struct Quadric
	{

		void setSphere(
			Vec3 center,
			Real radius);

		void setParabola(Vec3 origin,
			Vec3 direction,
			Real radius);

		void setTube(Vec3 origin,
			Vec3 direction,
			Real radius);

		void setCone(Vec3 origin,
			Vec3 direction,
			Real botRadius,
			Real topH,
			Real topRadius);

		// https://en.wikipedia.org/wiki/Conic_constant
		void setConicSurface(Vec3 origin,
			Vec3 direction,
			Real radiusOfCurvature,
			Real conicConstant);

		// https://en.wikipedia.org/wiki/Spheroid#Prolate_spheroids
		// (r . d)^2/a^2 +  ((r . n1)^2 + (r . n2))/b^2 = 1
		void setSpheroid(Vec3 origin,
			Vec3 direction,
			Real a, Real b);

		void shift(Vec3 r);



		static constexpr auto pars_ = pars::shape| pars::origin |
			pars::direction | pars::radius |
			pars::top_height | pars::top_radius;

		void set(pars::argument auto const &... args)
		{
			pars::check(pars_, args...);
			set(pars::uncheck, args...);
		}

		void set(pars::uncheck_t, pars::argument auto const &... args)
		{
			constexpr bool a = pars::has<decltype(args)...>(pars::origin);
			constexpr bool b = pars::has<decltype(args)...>(pars::direction);
			constexpr bool c = pars::has<decltype(args)...>(pars::radius);
			constexpr bool d = pars::has<decltype(args)...>(pars::shape);

			static_assert(d || (!a && !b && !c),
				"set origin, direction, or radius, but don't set shape");
			if constexpr (pars::has<decltype(args)...>(pars::shape)) {
				ShapeType type =
					pars::get(pars::shape, args...);
				if (type == ShapeType::Parabola) {
					if constexpr (b && c) {
						Vec3 origin = pars::get_default(pars::origin, Vec3{ 0,0,0 }, args...);
						setParabola(origin,
							pars::get(pars::direction, args...),
							pars::get(pars::radius, args...));
					}
					else {
						throw "";
					}
				}
				else if (type == ShapeType::Tube) {
					if constexpr (b && c) {
						Vec3 origin = pars::get_default(pars::origin, Vec3{ 0,0,0 }, args...);
						setTube(origin,
							pars::get(pars::direction, args...),
							pars::get(pars::radius, args...));
					}
					else {
						throw "";
					}
				}
				else if (type == ShapeType::Shpere) {
					if constexpr (c) {
						Vec3 origin = pars::get_default(pars::origin, Vec3{ 0,0,0 }, args...);
						setSphere(origin,
							pars::get(pars::radius, args...));
					}
					else {
						throw "";
					}
				} else if (type == ShapeType::Cone) {
					constexpr bool d = pars::has<decltype(args)...>(pars::top_height);
					constexpr bool e = pars::has<decltype(args)...>(pars::top_radius);

					if constexpr (b && c && d && e) {
						Vec3 origin = pars::get_default(pars::origin, Vec3{ 0,0,0 }, args...);
						Vec3 direction = pars::get(pars::direction, args...);
						setCone(origin, direction,
							pars::get(pars::radius, args...),
							pars::get(pars::top_height, args...),
							pars::get(pars::top_radius, args...)
						);
					} else {
						throw "";
					}
				}
				else {
					throw "";
				}
			}
		}

		bool inner(Vec3 const& p) const;

		//f(x) = Q_{ij} x_i x_j + P_i x_i + R = 0
		// inner: f(x) < 0
		// outer: f(x) > 0
		SymMatrix3X3 fQ;
		Vec3 fP{};
		Real fR{};
	};






	inline SymMatrix3X3 SymMatrix3X3::operator-() const {
		return SymMatrix3X3{ -fM11, -fM12, -fM13,
		-fM22, -fM23, -fM33 };
	}

	inline SymMatrix3X3::SymMatrix3X3() {}
	inline SymMatrix3X3::SymMatrix3X3(Real m11, Real m12, Real m13,
		Real m22, Real m23, Real m33) {
		fM11 = m11;
		fM12 = m12;
		fM13 = m13;
		fM22 = m22;
		fM23 = m23;
		fM33 = m33;
	}

	inline Vec3 dot(SymMatrix3X3 const& q, Vec3 const& v) {
		return { q.fM11 * v.fX +
				q.fM12 * v.fY +
				q.fM13 * v.fZ,
				q.fM12 * v.fX +
				q.fM22 * v.fY +
				q.fM23 * v.fZ,
				q.fM13 * v.fX +
				q.fM23 * v.fY +
				q.fM33 * v.fZ,
		};
	}

}
#endif

