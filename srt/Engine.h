#pragma once

#include "Real.h"
#include "Vec3.h"
#include "Bitmap.h"
#include "Recorder.h"
#include "Source.h"
#include "Pars.h"
#include <string>
#include <memory>
#include <functional>
#include <random>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <thread>

namespace srt {

	extern Real gSmin;

	struct PictureOpts
	{

		using Pars = pars::Pars<
			pars::width_,
			pars::high_,
			pars::origin_,
			pars::antiAliasLevel_,
			pars::samplePerPixel_,
			pars::n1_,
			pars::n2_,
			pars::n1Min_,
			pars::n1Max_,
			pars::lookAt_,
			pars::apertureDiameter_,
			pars::fieldOfView_,
			pars::fieldOfView1_,
			pars::fieldOfView2_,
			pars::n2Min_,
			pars::n2Max_,
			pars::lightOrigin_,
			pars::mult_,
			pars::stdoutProgress_>;

		int Width = 500;
		int High = 500;

		// from here we look, center-of-camera
		Vec3 Origin = { 0,0,0 };
		// along direction (n2 X n1)
		Vec3 N1 = { 1,0,0 };
		Vec3 N2 = { 0,1,0 };
		// focal distance, not focal length
		Real FocalDistance = kInfity;
		Real ApertureDiameter = 0;

		// antiAliasLevel == 0, no antiAliasLevel
		// > 0, slow but with antiAlias
		int AntiAliasLevel = 0;
		int SamplePoints = 100;

		Real N1Min = -1;
		Real N1Max = 1;
		Real N2Min = -1;
		Real N2Max = 1;

		Vec3 LightOrigin = { 0,0,kInfity };

		// multiple threads?
		bool Mult = false;

		bool stdoutProgress = false;

		PictureOpts(pars::argument auto const &... args)
		{
			pars::check< Pars>(args...);
			set(args...);
		}

		void set(pars::argument auto const &... args)
		{
			pars::check< Pars>(args...);
			if constexpr (pars::has<decltype(args)...>(pars::n1)) {
				pars::set(N1, pars::n1, args...);
				N1 = normalize(N1);
			}
			if constexpr (pars::has<decltype(args)...>(pars::n2)) {
				pars::set(N2, pars::n2, args...);
				N2 = normalize(N2);
			}

			pars::set(Width, pars::width, args...);
			pars::set(High, pars::high, args...);
			pars::set(Origin, pars::origin, args...);
			pars::set(AntiAliasLevel, pars::antiAliasLevel, args...);
			pars::set(Mult, pars::mult, args...);
			pars::set(SamplePoints, pars::samplePerPixel, args...);
			pars::set(N1Min, pars::n1Min, args...);
			pars::set(N1Max, pars::n1Max, args...);
			pars::set(N2Min, pars::n2Min, args...);
			pars::set(N2Max, pars::n2Max, args...);
			pars::set(LightOrigin, pars::lightOrigin, args...);
			pars::set(stdoutProgress, pars::stdoutProgress, args...);
			pars::set(ApertureDiameter, pars::apertureDiameter, args...);
			if constexpr (pars::has<decltype(args)...>(pars::lookAt)) {
				lookAt(pars::get(pars::lookAt, args...));
			}
			if constexpr (pars::has<decltype(args)...>(pars::fieldOfView)) {
				setFieldOfView1(pars::get(pars::fieldOfView, args...));
				setFieldOfView2(pars::get(pars::fieldOfView, args...));
			}
			if constexpr (pars::has<decltype(args)...>(pars::fieldOfView1)) {
				setFieldOfView1(pars::get(pars::fieldOfView1, args...));
			}
			if constexpr (pars::has<decltype(args)...>(pars::fieldOfView2)) {
				setFieldOfView2(pars::get(pars::fieldOfView2, args...));
			}


		}

		void setFieldOfView(Real s)
		{
			setFieldOfView1(s);
			setFieldOfView2(s);
		}

		void setFieldOfView1(Real s)
		{
			N1Min = -s / 2;
			N1Max = s / 2;
		}
		void setFieldOfView2(Real s)
		{
			N2Min = -s / 2;
			N2Max = s / 2;
		}

		void setOrigin(Vec3 o)
		{
			Origin = o;
		}

		// set n1 and n2, where
		// n1 _|_ n2, n1 _|_ (o - origin), n2 _|_ (o - origin)
		void lookAt(Vec3 o);

	};

	 struct Engine {

		void emit(Ray const& ray);
		void emit(int N);
		virtual void devicesPicture(std::string const& filename,
			PictureOpts const& opts);

		virtual void devicesPicture(Bitmap&,
			PictureOpts const& opts);
		virtual Bitmap devicesPicture(PictureOpts const& opts);

		void addRecorder(std::shared_ptr<Recorder> recorder)
		{
			fRecorders.push_back(recorder.get());
			fRecorders_.push_back(recorder);
		}
		void addSource(std::shared_ptr<Source> src)
		{
			fSources.push_back(src.get());
			fSources_.push_back(src);
		}

		void addDevice(std::shared_ptr<Device> dev)
		{
			fDevices.push_back(dev.get());
			fDevices_.push_back(dev);
		}

		Device* findDevice(std::string_view name);

		Bitmap eye(PictureOpts const& opts);

		std::vector<Device*>& getDevices() { return fDevices; }
	private:
		void doEmit(int N, Source& src);

		bool fSourceEqualChance = false;
		std::vector<Device*> fDevices;
		std::vector<std::shared_ptr<Device>> fDevices_;
		std::vector<Recorder*> fRecorders;
		std::vector<std::shared_ptr<Recorder>> fRecorders_;
		std::vector<Source*> fSources;
		std::vector<std::shared_ptr<Source>> fSources_;

		Source* fEye = nullptr;
		int fMaxLevel = 1000;
		double fMinAmp = 0.;
	};

}
