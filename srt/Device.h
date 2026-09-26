#ifndef SRT_DEVICE_H
#define SRT_DEVICE_H
#include "Pars.h"
#include "Real.h"
#include <string>
#include "Vec3.h"
#include "Ray.h"
#include "SurfaceProperties.h"

namespace srt {
	struct Device;

	// The nearest hit of a ray on a device, as Device::intersect finds it:
	// only what is needed to pick the nearest device and to shade it later.
	struct Hit {
		// distance along the ray
		Real t = kInfity;
		// whether the ray comes from the inner side
		bool in2out = false;
		// which part of the device, for its shade(): a face, ...
		int part = -1;
		// the device that shades the hit, if not the one intersected (e.g. the
		// face of a Convex); set by the innermost device that knows it
		Device const* sub = nullptr;
	};

	// Everything about the hit a ray is traced on: filled by Device::shade.
	struct TracingHandler {

		bool pictrue = false;
		bool record = false;


		bool hit = false;
		// inner to outer?
		Vec3 inter = {};
		bool inner = false;
		Vec3 N = {};
		SurfaceProperties const* property = nullptr;
		Device const* device = nullptr;

		void hitSurface(Vec3 inter,
			Vec3 N,
			bool inner,
			SurfaceProperties const* property,
			Device const* device)
		{
			this->hit = true;
			this->inter = inter;
			this->N = N;
			this->property = property;
			this->inner = inner;
			this->device = device;
		}
	};
	
	struct Device
	{
		std::string const& getName() const;
		void setName(std::string name);

		static constexpr auto pars_ = pars::empty | pars::name;

		void set(pars::uncheck_t, pars::argument auto const & ...args)
		{
			pars::set(fName, pars::name, args...);
		}

		void set(pars::argument auto const & ...args)
		{
			pars::check(pars_, args...);
			set(pars::uncheck, args...);
		}

		// the nearest hit of ray ahead of it (beyond gSmin) and nearer than
		// tMax: fills hit and returns true, or returns false
		virtual bool intersect(Ray const& ray, Real tMax, Hit& hit) const = 0;
		// the full hit (point, normal, material) for a hit that intersect()
		// found on this device
		virtual void shade(Ray const& ray, Hit const& hit, TracingHandler& out) const = 0;

		bool dependsOnWavelength() const
		{
			return fWavelength == WavelengthUse::Tracing;
		}

		// what shade() does with the ray's wavelength
		enum class WavelengthUse {
			// nothing
			None,
			// reads it in shade(): such a device only gets rays with
			// fLambda != 0 (see Ray::fLambda); its intersect() must not
			// depend on the wavelength
			Tracing,
		};
		WavelengthUse fWavelength = WavelengthUse::None;

		// which kind of device this is, so that the nearest-hit search can call
		// the common ones without a virtual call (and inline them)
		enum class Kind {
			Other,
			Plane,		// PlaneSurface
			Quadric,	// QuadricSurface
			Convex,		// Convex
			ConvexPolyhedron,	// ConvexPolyhedron
			Polyhedron,	// Polyhedron
		};
		Kind fKind = Kind::Other;
	private:
		// not used
		std::string fName;
	};


	inline void Device::setName(std::string name)
	{
		fName = std::move(name);
	}

	inline std::string const& Device::getName() const
	{
		return fName;
	}

}
#endif
