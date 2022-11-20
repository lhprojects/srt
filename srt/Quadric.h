#ifndef SRT_QUADRIC_H
#define SRT_QUADRIC_H

#include "Vec3.h"
#include "Real.h"
#include "Pars.h"

namespace srt {

	struct SymMatrix3X3 {

		SymMatrix3X3();

		Real fM11{};
		Real fM12{};
		Real fM13{};
		Real fM22{};
		Real fM23{};
		Real fM33{};

	};

	inline SymMatrix3X3::SymMatrix3X3() {}

	inline Vec3 dot(SymMatrix3X3 const& q, Vec3 const& v)
	{
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

	struct Sphere {
		using Pars = pars::Pars<pars::shape_,
			pars::origin_,
			pars::radius_>;

		void setSphere(
			Vec3 center,
			Real radius);
		bool inner(Vec3 const& p) const;

		void set(pars::argument auto const &... args) {
			pars::check<Pars>(args...);
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

		using Pars = pars::Pars<pars::shape_, pars::origin_,
			pars::direction_, pars::radius_>;

		void set(pars::argument auto const &... args)
		{
			pars::check<Pars>(args...);
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
					if constexpr (a && b && c) {
						setParabola(pars::get(pars::origin, args...),
							pars::get(pars::direction, args...),
							pars::get(pars::radius, args...));
					}
					else {
						throw "";
					}
				}
				else if (type == ShapeType::Tube) {
					if constexpr (a && b && c) {
						setTube(pars::get(pars::origin, args...),
							pars::get(pars::direction, args...),
							pars::get(pars::radius, args...));
					}
					else {
						throw "";
					}
				}
				else if (type == ShapeType::Shpere) {
					if constexpr (a && c) {
						setSphere(pars::get(pars::origin, args...),
							pars::get(pars::radius, args...));
					}
					else {
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


}
#endif
