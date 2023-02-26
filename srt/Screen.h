#include <iostream>
#include <fstream>
#include "Real.h"
#include "Align.h"
#include "Iter.h"
#include "Ray.h"
#include "Pars.h"
#include "Device.h"
#include "Surface.h"
#include "Scaler.h"
#include "wavelength.h"
#include "Bitmap.h"
#include "Surfaces.h"

namespace srt {
	
	struct ScreenOpts
	{

		using Pars = pars::Pars<
			pars::high_,
			pars::width_,
			pars::n1_,
			pars::n2_,
			pars::origin_,
			pars::screenSize_,
			pars::n1Min_,
			pars::n2Min_,
			pars::n1Max_,
			pars::n2Max_,
			pars::gray_>;

		ScreenOpts(pars::argument auto const &... args)
		{
			set(args...);
		}

		void set(pars::argument auto const &... args)
		{
			pars::check<Pars>(args...);
			pars::set(High, pars::high, args...);
			pars::set(Width, pars::width, args...);
			pars::set(N1, pars::n1, args...);
			pars::set(N2, pars::n2, args...);
			pars::set(Origin, pars::origin, args...);
			if constexpr (pars::has<decltype(args)...>(pars::screenSize)) {
				setScreenSize(get(pars::screenSize, args...));
			}
			pars::set(N1Min, pars::n1Min, args...);
			pars::set(N2Min, pars::n2Min, args...);
			pars::set(N1Max, pars::n1Max, args...);
			pars::set(N2Max, pars::n2Max, args...);
			pars::set(Gray, pars::gray, args...);

		}

		int Width = 500;
		int High = 500;

		Vec3 N1 = { 1,0,0 };
		Vec3 N2 = { 0,1,0 };
		Vec3 Origin = { 0,0,0 };

		Real N1Min = -1;
		Real N1Max = 1;
		Real N2Min = -1;
		Real N2Max = 1;

		bool Gray = false;
		void setScreenSize(Real s)
		{
			N1Min = -s / 2;
			N1Max = s / 2;
			N2Min = -s / 2;
			N2Max = s / 2;
		}
	};

	struct Raster
	{
		void raster(Bitmap&,
			Iter<Ray>& iter,
			ScreenOpts const& opts);
	};

	struct Screen {

		Screen() {}

		static constexpr auto pars_ = pars::record | pars::recordIn2Out | pars::recordOut2In;

		void set(pars::argument auto const &... args)
		{
			pars::check(pars_, args...);
			set(args...);
		}

		void set(pars::uncheck_t, pars::argument auto const &... args)
		{
			pars::set(fRecordIn2Out, pars::record, args...);
			pars::set(fRecordOut2In, pars::record, args...);
			pars::set(fRecordIn2Out, pars::recordIn2Out, args...);
			pars::set(fRecordOut2In, pars::recordOut2In, args...);
		}

		void recordIn2Out(bool r) { fRecordIn2Out = r; }
		void recordOut2In(bool r) { fRecordOut2In = r; }

		// file type infered from filename
		void save(std::string const& filename);

		void raster(std::string const& filename, ScreenOpts const& opts);
		void raster(std::ostream& os, ImageFileFormat iff, ScreenOpts const& opts);
		void raster(Bitmap&, ScreenOpts const& opts);

		void record(
			Ray const& in, ProcessHandler& handler) const;

	private:
		bool fRecordIn2Out = true;
		bool fRecordOut2In = true;
		mutable std::vector<Ray> fRays;

	};

	struct PlaneScreen : PlaneSurface, Screen
	{
		PlaneScreen() = default;

		static constexpr auto pars_ = PlaneSurface::pars_ | Screen::pars_;

		PlaneScreen(pars::argument auto const &... args)
		{
			pars::check(pars_, args...);
			// by default screen is non-refractive and non-reflective
			set(pars::uncheck, pars::in2OutRefractRatio = 0);
			set(pars::uncheck, pars::out2InRefractRatio = 0);
			set(pars::uncheck, pars::innerReflectRatio = 0);
			set(pars::uncheck, pars::outerReflectRatio = 0);
			set(pars::uncheck, args...);
		}

		PlaneScreen(pars::uncheck_t, pars::argument auto const &... args)
		{
			// by default screen is non-refractive and non-reflective
			set(pars::uncheck, pars::in2OutRefractRatio = 0);
			set(pars::uncheck, pars::out2InRefractRatio = 0);
			set(pars::uncheck, pars::innerReflectRatio = 0);
			set(pars::uncheck, pars::outerReflectRatio = 0);
			set(pars::uncheck, args...);
		}

		void set(pars::argument auto const &... args)
		{
			pars::check(pars_, args...);
			set(pars::uncheck, args...);
		}

		void set(pars::uncheck_t, pars::argument auto const &... args)
		{
			PlaneSurface::set(pars::uncheck, args...);
			Screen::set(pars::uncheck, args...);
		}


		void process(Ray const& in, ProcessHandler& handler) const override;
	};

	std::shared_ptr<PlaneScreen> planeScreen(pars::argument auto const &... args)
	{
		return std::make_shared<PlaneScreen>(args...);
	}

	struct QuadricScreen : QuadricSurface, Screen
	{
		template<class... Args>
		void set(Args const &... args)
		{
			QuadricSurface::set(args...);
			Screen::set(args...);
		}
		QuadricScreen();
		template<class... Args>
		QuadricScreen(Args const &... args)
		{
			set(args...);
		}

		void process(Ray const& in, ProcessHandler& handler) const override;
	};


}
