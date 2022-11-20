#ifndef SRT_PLANE_H
#define SRT_PLANE_H
#include "Pars.h"

namespace srt {

	struct Plane
	{

		using Pars = pars::Pars<pars::origin_, pars::direction_>;

		Plane() = default;

		Plane(pars::uncheck_t, pars::argument auto const &... args)
		{
			set(pars::uncheck, args...);
		}

		Plane(pars::argument auto const &... args)
		{
			pars::check<Pars>(args...);
			set(pars::uncheck, args...);
		}

		void set(pars::argument auto const &... args)
		{
			pars::check<Pars>(args...);
			set(pars::uncheck, args...);
		}

		void set(pars::uncheck_t, pars::argument auto const &... args)
		{
			if constexpr (pars::has<decltype(args)...>(pars::origin)
				|| pars::has<decltype(args)...>(pars::direction)) {
				static_assert(pars::has<decltype(args)...>(pars::origin), "must have origin");
				static_assert(pars::has<decltype(args)...>(pars::direction), "must have direction");
				Vec3 o = pars::get(pars::origin, args...);
				Vec3 p = pars::get(pars::direction, args...);
				setOP(o, p);
			}
		}

		// set this as new plane:
		// 1. O is on the plane
		// 2. P is normal to the plane
		void setOP(Vec3 const& o, Vec3 const& p);
		// if p is inner
		bool inner(Vec3 const& p) const;
		// if p is outer
		bool outer(Vec3 const& p) const;
		//
		//f(x) = P_i x_i + R = 0
		// inner: f(x) < 0
		// outer: f(x) < 0
		Vec3 fP{};
		Real fR{};
	};


	inline bool Plane::inner(Vec3 const& p) const
	{
		return dot(fP, p) + fR < 0;
	}

	inline bool Plane::outer(Vec3 const& p) const
	{
		return dot(fP, p) + fR > 0;
	}

	inline void Plane::setOP(Vec3 const& O, Vec3 const& P)
	{
		fP = P;
		fR = dot(-O, P);
	}

}
#endif
