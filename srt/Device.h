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
	 enum class HandlerType {
		Distance,
		Tracing,
	};

	 struct ProcessHandler {
		HandlerType fType;

		constexpr bool isDistancing() const {
			return fType == HandlerType::Distance;
		}
	};

	 struct DistanceHandler : ProcessHandler {

		DistanceHandler()
		{
			fType = HandlerType::Distance;
		}
		Real fDistance = kInfity;
		bool fIn2out = false;

		void distance(Real s, bool in2out)
		{
			fDistance = s;
			fIn2out = in2out;
		}
	};

	 struct TracingHandler : ProcessHandler {

		TracingHandler()
		{
			fType = HandlerType::Tracing;
		}

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

		virtual void process(Ray const& in, ProcessHandler& handler) const = 0;

		bool dependsOnWavelength() const
		{
			return fWavelength == WavelengthUse::Tracing;
		}

		// what process() does with the ray's wavelength
		enum class WavelengthUse {
			// nothing
			None,
			// reads it with a TracingHandler: such a device only gets rays with
			// fLambda != 0 (see Ray::fLambda); its distance pass must not
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
