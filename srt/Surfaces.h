#ifndef SRT_SURFACES_H
#define SRT_SURFACES_H

#include <memory>
#include <vector>
#include <format>
#include "Surface.h"
#include "Ray.h"
#include "Plane.h"
#include "Quadric.h"
#include "Engine.h"
#include "Bound.h"

namespace srt {


	struct QuadricSurface : Surface, Quadric {

		static constexpr auto pars_ = Surface::pars_ | Quadric::pars_;

		QuadricSurface(pars::argument auto const &... args) {
			fKind = Kind::Quadric;
			set(args...);
		}

		QuadricSurface(pars::uncheck_t, pars::argument auto const &... args) {
			fKind = Kind::Quadric;
			set(pars::uncheck, args...);
		}

		void set(pars::argument auto const &... args) {
			pars::check(pars_, args...);
			set(pars::uncheck, args...);
		}

		void set(pars::uncheck_t, pars::argument auto const &... args) {
			Surface::set(pars::uncheck, args...);
			Quadric::set(pars::uncheck, args...);
		}

		bool isInner(Vec3 const& p) const override {
			return inner(p);
		}

		inline bool intersect(Ray const& ray, Real tMax, Hit& hit) const override;
		inline void shade(Ray const& ray, Hit const& hit, TracingHandler& out) const override;
	};

	std::shared_ptr<QuadricSurface> quadricSurface(pars::argument auto const &... args) {
		pars::check(QuadricSurface::pars_, args...);
		return std::make_shared<QuadricSurface>(args...);
	}

	inline bool QuadricSurface::intersect(Ray const& r, Real tMax,
		Hit& hit) const
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
			return false;
		}
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
				// the gradient points to the outer side
				hit.in2out = dot(fP + 2. * dot(fQ, inter), D) > 0;
				return true;
			}
		}
		return false;
	}

	inline void QuadricSurface::shade(Ray const& r, Hit const& hit,
		TracingHandler& out) const
	{
		Vec3 inter = r.fO + r.fD * hit.t;
		Vec3 N = normalize(fP + 2. * dot(fQ, inter));
		out.hitSurface(inter, N, hit.in2out, this, this);
	}

	
	std::string to_string(QuadricSurface const& q);

}

namespace std {


	template<class T, class CharT>
	struct tostringfmt {
		std::formatter<std::string> strfmt;

		constexpr auto parse(auto& pc) {
			return strfmt.parse(pc);
		}

		auto format(T const& p, auto& fc) const {
			return strfmt.format(to_string(p), fc);
		}

	};

	template<class CharT>
	struct formatter<srt::QuadricSurface, CharT> : tostringfmt<srt::QuadricSurface, CharT> {
	};
	template<class CharT>
	struct formatter<srt::SymMatrix3X3, CharT> : tostringfmt<srt::SymMatrix3X3, CharT> {
	};
	template<class CharT>
	struct formatter<srt::Vec3, CharT> : tostringfmt<srt::Vec3, CharT> {
	};

}


namespace srt {

	// not test yet! don't use!
	// not test yet! don't use!
	// not test yet! don't use!
 	struct SphereSurface : Surface, Sphere {

		static constexpr auto pars_ = Surface::pars_ | Sphere::pars_;

		SphereSurface(pars::argument auto const &... args) {
			set(args...);
		}

		SphereSurface(pars::uncheck_t, pars::argument auto const &... args) {
			set(pars::uncheck, args...);
		}

		void set(pars::argument auto const &... args) {
			pars::check(pars_, args...);
			set(pars::uncheck, args...);
		}

		void set(pars::uncheck_t, pars::argument auto const &... args) {
			Surface::set(pars::uncheck, args...);
			Sphere::set(pars::uncheck, args...);
		}

		bool isInner(Vec3 const& p) const override {
			return inner(p);
		}

		bool intersect(Ray const& ray, Real tMax, Hit& hit) const override;
		void shade(Ray const& ray, Hit const& hit, TracingHandler& out) const override;
	};


	struct PlaneSurface : Surface, Plane
	{
		PlaneSurface() { fKind = Kind::Plane; }

		static constexpr auto pars_ = Surface::pars_ | Plane::pars_;

		PlaneSurface(pars::argument auto const &... args)
		{
			fKind = Kind::Plane;
			set(args...);
		}

		void set(pars::uncheck_t, pars::argument auto const &... args)
		{
			Surface::set(pars::uncheck, args...);
			Plane::set(pars::uncheck, args...);
		}

		void set(pars::argument auto const &... args)
		{
			pars::check(pars_, args...);
			Surface::set(pars::uncheck, args...);
			Plane::set(pars::uncheck, args...);
		}

		bool isInner(Vec3 const& p) const override;
		void setGridTexture(Real w);
		bool intersect(Ray const& r, Real tMax, Hit& hit) const override
		{
			// <P,O+Ds> + R = 0
			Real b = dot(fP, r.fO) + fR;
			Real a = dot(fP, r.fD);
			// a s + b = 0
			if (a * b > 0) {
				return false;
			}
			Real s = -b / a;
			if (!(s > gSmin)) {
				// surface is behind ray
				return false;
			}
			if (s >= tMax || !inBound(getBound(), r.fO + r.fD * s)) {
				return false;
			}
			hit.t = s;
			hit.in2out = b < 0;
			return true;
		}

		void shade(Ray const& r, Hit const& hit, TracingHandler& out) const override
		{
			out.hitSurface(r.fO + r.fD * hit.t, normalize(fP), hit.in2out,
				this, this);
		}
	};

	std::shared_ptr<PlaneSurface> planeSurface(pars::argument auto const &... args)
	{
		pars::check(PlaneSurface::pars_, args...);
		return std::make_shared<PlaneSurface>(args...);
	}

	struct ShiftSurface : Surface
	{

		ShiftSurface(std::shared_ptr<Surface> sur, Vec3 s);
		bool intersect(Ray const& ray, Real tMax, Hit& hit) const override;
		void shade(Ray const& ray, Hit const& hit, TracingHandler& out) const override;
		bool isInner(Vec3 const& p) const override;
		void shift(Vec3 const &p);
	private:
		Vec3 fShift;
		std::shared_ptr<Surface> fOrigin;
	};

	std::shared_ptr<ShiftSurface> shift(std::shared_ptr<Surface> sur, Vec3 const& s);

	inline std::shared_ptr<QuadricSurface> inverse(std::shared_ptr<QuadricSurface> sur)
	{
		auto newsur = std::make_shared<QuadricSurface>(*sur);
		newsur->fP = -sur->fP;
		newsur->fR = -sur->fR;
		newsur->fQ = -sur->fQ;
		return newsur;
	}
	
	inline std::shared_ptr<PlaneSurface> inverse(std::shared_ptr<PlaneSurface> sur)
	{
		auto newsur = std::make_shared<PlaneSurface>(*sur);
		newsur->fP = -sur->fP;
		newsur->fR = -sur->fR;
		return newsur;
	}

}

#endif
