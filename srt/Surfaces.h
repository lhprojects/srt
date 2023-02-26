#ifndef SRT_SURFACES_H
#define SRT_SURFACES_H

#include <memory>
#include <vector>
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
			set(args...);
		}

		QuadricSurface(pars::uncheck_t, pars::argument auto const &... args) {
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
	};

	std::shared_ptr<QuadricSurface> quadricSurface(pars::argument auto const &... args) {
		pars::check(QuadricSurface::pars_, args...);
		return std::make_shared<QuadricSurface>(args...);
	}
	
	std::string to_string(QuadricSurface const& q);

}

namespace std {


	template<class T, class CharT>
	struct tostringfmt {
		std::formatter<std::string> strfmt;

		auto parse(auto& pc) {
			return strfmt.parse(pc);
		}

		auto format(T const& p, auto& fc) {
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
			pars::check(pars, args...);
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
		PlaneSurface() = default;

		static constexpr auto pars_ = Surface::pars_ | Plane::pars_;

		PlaneSurface(pars::argument auto const &... args)
		{
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
	};

	std::shared_ptr<PlaneSurface> planeSurface(pars::argument auto const &... args)
	{
		pars::check(PlaneSurface::pars_, args...);
		return std::make_shared<PlaneSurface>(args...);
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

	inline std::shared_ptr<ShiftSurface> inverse(std::shared_ptr<QuadricSurface> sur)
	{
		auto newsur = std::make_shared<QuadricSurface>(*sur);
		newsur->fP = -sur->fP;
		newsur->fR = -sur->fR;
		newsur->fQ = -sur->fQ;
	}
	
	inline std::shared_ptr<PlaneSurface> inverse(std::shared_ptr<PlaneSurface> sur)
	{
		auto newsur = std::make_shared<PlaneSurface>(*sur);
		newsur->fP = -sur->fP;
		newsur->fR = -sur->fR;
	}

}

#endif
