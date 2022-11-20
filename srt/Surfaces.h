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


	struct QuadricSurface : Surface, Quadric
	{

		using Pars = pars::MergePars<Surface::Pars, Quadric::Pars>;

		QuadricSurface(pars::argument auto const &... args)
		{
			set(args...);
		}

		QuadricSurface(pars::uncheck_t, pars::argument auto const &... args)
		{
			set(pars::uncheck, args...);
		}

		void set(pars::argument auto const &... args)
		{
			pars::check<Pars>(args...);
			set(pars::uncheck, args...);
		}

		void set(pars::uncheck_t, pars::argument auto const &... args)
		{
			Surface::set(pars::uncheck, args...);
			Quadric::set(pars::uncheck, args...);
		}

		bool isInner(Vec3 const& p) const override
		{
			return inner(p);
		}

		void process(Ray const& in, ProcessHandler& handler) const override;
	};

	std::shared_ptr<QuadricSurface> quadricSurface(pars::argument auto const &... args)
	{
		pars::check<QuadricSurface::Pars>(args...);
		return std::make_shared<QuadricSurface>(args...);
	}

	// not test yet! don't use!
	// not test yet! don't use!
	// not test yet! don't use!
 	struct SphereSurface : Surface, Sphere {

		using Pars = pars::MergePars<Surface::Pars, Sphere::Pars>;

		SphereSurface(pars::argument auto const &... args) {
			set(args...);
		}

		SphereSurface(pars::uncheck_t, pars::argument auto const &... args) {
			set(pars::uncheck, args...);
		}

		void set(pars::argument auto const &... args) {
			pars::check<Pars>(args...);
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

		using Pars = pars::MergePars<Surface::Pars,
			Plane::Pars>;

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
			pars::check<Pars>(args...);
			Surface::set(pars::uncheck, args...);
			Plane::set(pars::uncheck, args...);
		}

		bool isInner(Vec3 const& p) const override;
		void setGridTexture(Real w);
		void process(Ray const& r, ProcessHandler& handler) const override;
	};

	std::shared_ptr<PlaneSurface> planeSurface(pars::argument auto const &... args)
	{
		pars::check<PlaneSurface::Pars>(args...);
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
}

#endif
