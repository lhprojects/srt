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

		void process(Ray const& in, ProcessHandler& handler) const override;

		// the nearest hit ahead of r within the bound: its distance s, point
		// and normal
		inline bool intersect(Ray const& r, Real& s, Vec3& inter, Vec3& N) const;

		// the distance pass of process(): s = kInfity on a miss
		void distance(Ray const& r, Real& s, bool& in2out) const
		{
			Vec3 inter, N;
			if (intersect(r, s, inter, N)) {
				in2out = dot(N, r.fD) > 0;
			} else {
				s = kInfity;
			}
		}
	};

	std::shared_ptr<QuadricSurface> quadricSurface(pars::argument auto const &... args) {
		pars::check(QuadricSurface::pars_, args...);
		return std::make_shared<QuadricSurface>(args...);
	}

	inline bool QuadricSurface::intersect(Ray const& r, Real& s,
		Vec3& inter, Vec3& N) const
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

		if (s1 <= gSmin/* && s2 > 0*/) {
			inter = r.fO + r.fD * s2;
			if (!inBound(getBound(), inter)) {
				return false;
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
					return false;
				}
			}
		}

		N = normalize(fP + 2. * dot(fQ, inter));
		return true;
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

		void process(Ray const& in, ProcessHandler& handler) const override;
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
		void process(Ray const& r, ProcessHandler& handler) const override;

		// the hit ahead of r within the bound: its distance s and point;
		// in2out: whether r comes from the inner side
		bool intersect(Ray const& r, Real& s, bool& in2out, Vec3& inter) const
		{
			// <P,O+Ds> + R = 0
			Real b = dot(fP, r.fO) + fR;
			Real a = dot(fP, r.fD);
			// a s + b = 0
			if (a * b > 0) {
				return false;
			}
			Real s1 = -b / a;
			if (s1 <= gSmin) {
				// surface is behind ray
				return false;
			}
			inter = r.fO + r.fD * s1;
			if (!inBound(getBound(), inter)) {
				return false;
			}
			s = s1;
			in2out = b < 0;
			return true;
		}

		// the distance pass of process(): s = kInfity on a miss
		void distance(Ray const& r, Real& s, bool& in2out) const
		{
			Vec3 inter;
			if (!intersect(r, s, in2out, inter)) {
				s = kInfity;
			}
		}
	};

	std::shared_ptr<PlaneSurface> planeSurface(pars::argument auto const &... args)
	{
		pars::check(PlaneSurface::pars_, args...);
		return std::make_shared<PlaneSurface>(args...);
	}

	inline void PlaneSurface::process(Ray const& r, ProcessHandler& handler) const
	{
		Real s;
		bool in2out;
		Vec3 inter;
		if (!intersect(r, s, in2out, inter)) {
			return;
		}
		if (handler.fType == HandlerType::Distance) {
			static_cast<DistanceHandler&>(handler).distance(s, in2out);
		} else if (handler.fType == HandlerType::Tracing) {
			Vec3 N = normalize(fP);
			static_cast<TracingHandler&>(handler).hitSurface(inter,
				N, in2out, this, this);
		}
	}

	struct ShiftSurface : Surface
	{

		ShiftSurface(std::shared_ptr<Surface> sur, Vec3 s);
		void process(Ray const& r, ProcessHandler& handler) const override;
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
