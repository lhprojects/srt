#pragma once
#include "Pars.h"
#include <memory>
#include <vector>
#include "Bound.h"
#include "Quadric.h"
#include "Plane.h"
#include "Surface.h"

namespace srt {
	
	struct BoxBound : Bound
	{

		BoxBound(Real x0, Real x1, Real y0, Real y1, Real z0, Real z1)
		{
			fX0 = x0;
			fX1 = x1;
			fY0 = y0;
			fY1 = y1;
			fZ0 = z0;
			fZ1 = z1;
		}

		Real fX0 = -kInfity;
		Real fX1 = +kInfity;
		Real fY0 = -kInfity;
		Real fY1 = +kInfity;
		Real fZ0 = -kInfity;
		Real fZ1 = +kInfity;

		void setXBound(Real x0, Real x1) { fX0 = x0; fX1 = x1; }
		void setYBound(Real x0, Real x1) { fY0 = x0; fY1 = x1; }
		void setZBound(Real x0, Real x1) { fZ0 = x0; fZ1 = x1; }

		virtual bool onInBound(Vec3 const& p) const
		{
			return p.fX > fX0 && p.fX < fX1&&
				p.fY > fY0 && p.fY < fY1&&
				p.fZ > fZ0 && p.fZ < fZ1;
		}
	};

	inline std::shared_ptr<Bound> boxBound(Real x0, Real x1, Real y0, Real y1, Real z0, Real z1)
	{
		return std::make_shared<BoxBound>(x0, x1, y0, y1, z0, z1);
	}

	inline std::shared_ptr<Bound> boxBound(pars::argument auto const &... args)
	{
		using Pars = pars::Pars<
			pars::x0_, pars::x1_, pars::y0_, pars::y1_, pars::z0_, pars::z1_>;

		pars::check<Pars>(args...);
		Real x0, x1, y0, y1, z0, z1;
		x0 = pars::get_default(pars::x0, -kInfity, args...);
		x1 = pars::get_default(pars::x1, +kInfity, args...);
		y0 = pars::get_default(pars::y0, -kInfity, args...);
		y1 = pars::get_default(pars::y1, +kInfity, args...);
		z0 = pars::get_default(pars::z0, -kInfity, args...);
		z1 = pars::get_default(pars::z1, +kInfity, args...);

		return boxBound(x0, x1, y0, y1, z0, z1);
	}


	struct AllBound : Bound {

		bool onInBound(Vec3 const& p) const override;
		void addBound(std::shared_ptr<Bound> b);

		std::vector<std::shared_ptr<Bound>> fBounds;
	};

	struct AnyBound : Bound {

		void addBound(std::shared_ptr<Bound> b);
		bool onInBound(Vec3 const& p) const override;

		std::vector<std::shared_ptr<Bound>> fBounds;
	};

	struct InverseBound : Bound {

		InverseBound() = default;
		InverseBound(std::shared_ptr<Bound> b);
		bool onInBound(Vec3 const& p) const override;
		void setBound(std::shared_ptr<Bound> b);

		std::shared_ptr<Bound> fBound;
	};

	std::shared_ptr<AllBound> all(std::initializer_list<std::shared_ptr<Bound>> bounds);
	std::shared_ptr<AnyBound> any(std::initializer_list<std::shared_ptr<Bound>> bounds);
	std::shared_ptr<InverseBound> inverse(std::shared_ptr<Bound> bound);

	struct QuadricBound : Bound, Quadric
	{

		using Pars = Quadric::Pars;

		QuadricBound(pars::argument auto const &... args)
		{
			Quadric::set(args...);
		}

		QuadricBound(pars::uncheck_t, pars::argument auto const &... args)
		{
			Quadric::set(pars::uncheck, args...);
		}

		bool onInBound(Vec3 const& p) const override
		{
			return inner(p);
		}
	};

	 std::shared_ptr<QuadricBound> quadricBound(pars::argument auto const &... args)
	{
		pars::check<QuadricBound::Pars>(args...);
		return std::make_shared<QuadricBound>(args...);
	}

	struct PlaneBound : Bound, Plane
	{

		PlaneBound(pars::argument auto const &...args) :
			Plane(args...)
		{
		}

		PlaneBound() = default;
		bool onInBound(Vec3 const& p) const override;
	};

	inline bool PlaneBound::onInBound(Vec3 const& p) const
	{
		return inner(p);
	}

	inline std::shared_ptr<PlaneBound> planeBound(pars::argument auto const &... args)
	{
		pars::check<QuadricBound::Pars>(args...);
		return std::make_shared<PlaneBound>(args...);
	}

	struct SurfaceBound : Bound
	{
		SurfaceBound(std::shared_ptr<Surface> ss);
		bool onInBound(Vec3 const& p) const override;
		std::shared_ptr<Surface> s;
	};
	std::shared_ptr<Bound> asBound(std::shared_ptr<Surface> s);

}