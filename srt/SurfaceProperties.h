#ifndef SRT_SURFACEPROPERTIES_H
#define SRT_SURFACEPROPERTIES_H

#include <memory>
#include "Real.h"
#include "Pars.h"
#include "Color.h"
#include "Real.h"
#include "Texture.h"
#include "ReflectType.h"

namespace srt
{
	enum class RefractiveIndexType {
		Const,
		Function,
	};

	struct UnitaryFunction
	{
		virtual Real operator()(Real lambda) = 0;
	};

	struct Sellmeier3Function : UnitaryFunction
	{
		Sellmeier3Function(Real const* bcs);

		Real B1, B2, B3, C1, C2, C3;

		Real operator()(Real lambda) override;
	};
	std::shared_ptr<Sellmeier3Function> Sellmeier3(PredefinedSellmeier3 p);

	struct RefractiveIndex
	{
		RefractiveIndexType fType;
		Real fValue;
		std::shared_ptr<UnitaryFunction> fFunction;

		RefractiveIndex(Real index) {
			set(index);
		}

		RefractiveIndex(RefractiveIndex const &r)
		{
			fType = r.fType;
			if (r.fType == RefractiveIndexType::Const) {
				fValue = r.fValue;
			}
			else {
				fFunction = r.fFunction;
			}
		}
		RefractiveIndex(RefractiveIndex && r)  noexcept
		{
			fType = r.fType;
			if (r.fType == RefractiveIndexType::Const) {
				fValue = r.fValue;
			}
			else {
				fFunction = std::move(r.fFunction);
			}
		}

		RefractiveIndex& operator=(RefractiveIndex const &r)
		{
			fType = r.fType;
			if (fType == RefractiveIndexType::Const) {
				fValue = r.fValue;
			}
			else {
				fFunction = r.fFunction;
			}
		}

		RefractiveIndex &operator=(RefractiveIndex&& r) noexcept
		{
			fType = r.fType;
			if (fType == RefractiveIndexType::Const) {
				fValue = r.fValue;
			}
			else {
				fFunction = std::move(r.fFunction);
			}
		}

		~RefractiveIndex() {}

		void set(Real index)
		{
			fType = RefractiveIndexType::Const;
			fValue = index;
		}

		void set(std::shared_ptr<UnitaryFunction> index)
		{
			fType = RefractiveIndexType::Function;
			fFunction = index;
		}

		Real get(Real lambda) const
		{
			if (fType == RefractiveIndexType::Const) {
				return fValue;
			} else  {
				return (*fFunction)(lambda);
			}
		}
	};

	struct SurfaceProperties
	{
		void setInnerIndex(Real index);
		void setOuterIndex(Real index);
		void setInnerIndex(std::shared_ptr<UnitaryFunction>);
		void setOuterIndex(std::shared_ptr<UnitaryFunction>);

		void setInnerTrans(Real v);
		void setOuterTrans(Real v);
		void setInnerReflect(Real v);
		void setOuterReflect(Real v);

		void setInnerTrans(std::shared_ptr<TextureInterface>);
		void setOuterTrans(std::shared_ptr<TextureInterface>);
		void setInnerReflect(std::shared_ptr<TextureInterface>);
		void setOuterReflect(std::shared_ptr<TextureInterface>);

		void setTrans(Real v);
		void setReflect(Real v);
		void setTrans(std::shared_ptr<TextureInterface>);
		void setReflect(std::shared_ptr<TextureInterface>);

		void setPictureColor(Color c);
		void setInnerPictureColor(Color c);
		void setOuterPictureColor(Color c);
		void setPictureAlpha(Real a);
		void setInnerPictureAlpha(Real a);

		void setInnerReflectType(ReflectType type);
		void setOuterReflectType(ReflectType type);
		void setReflectType(ReflectType type);

		void setBrightness(Real amp);

		static constexpr auto pars_ = pars::reflectRatio|
			pars::innerReflectRatio |
			pars::outerReflectRatio |
			pars::reflectType |
			pars::innerReflectType |
			pars::outerReflectType |
			pars::refractRatio |
			pars::in2OutRefractRatio |
			pars::out2InRefractRatio |
			pars::innerIndex |
			pars::outerIndex |
			pars::innerPredefinedSellmeier3 |
			pars::outerPredefinedSellmeier3 |
			pars::pictureColor |
			pars::innerPictureColor |
			pars::outerPictureColor |
			pars::brightness |
			pars::spectrum;

		void set(pars::argument auto const &... args)
		{
			pars::check(pars_, args...);
			set(pars::uncheck, args...);
		}

		void set(pars::uncheck_t, pars::argument auto const &... args)
		{
			if constexpr (pars::has<decltype(args)...>(pars::reflectRatio)) {
				setReflect(pars::get(pars::reflectRatio, args...));
			}
			if constexpr (pars::has<decltype(args)...>(pars::innerReflectRatio)) {
				setInnerReflect(pars::get(pars::innerReflectRatio, args...));
			}
			if constexpr (pars::has<decltype(args)...>(pars::outerReflectRatio)) {
				setOuterReflect(pars::get(pars::outerReflectRatio, args...));
			}

			pars::set(fInnerReflectType,
				pars::reflectType,
				args...);

			pars::set(fInnerReflectType,
				pars::innerReflectType,
				args...);

			pars::set(fOuterReflectType,
				pars::reflectType,
				args...);

			pars::set(fOuterReflectType,
				pars::outerReflectType,
				args...);

			if constexpr (pars::has<decltype(args)...>(pars::refractRatio)) {
				setTrans(pars::get(pars::refractRatio, args...));
			}
			if constexpr (pars::has<decltype(args)...>(pars::in2OutRefractRatio)) {
				setInnerTrans(pars::get(pars::in2OutRefractRatio, args...));
			}
			if constexpr (pars::has<decltype(args)...>(pars::out2InRefractRatio)) {
				setOuterTrans(pars::get(pars::out2InRefractRatio, args...));
			}




			if constexpr (pars::has<decltype(args)...>(pars::innerIndex)) {
				setInnerIndex(pars::get(pars::innerIndex, args...));
			}
			if constexpr (pars::has<decltype(args)...>(pars::outerIndex)) {
				setOuterIndex(pars::get(pars::outerIndex, args...));
			}
			if constexpr (pars::has<decltype(args)...>(pars::innerPredefinedSellmeier3)) {
				setInnerIndex(Sellmeier3(pars::get(pars::innerPredefinedSellmeier3, args...)));
			}
			if constexpr (pars::has<decltype(args)...>(pars::outerPredefinedSellmeier3)) {
				setOuterIndex(Sellmeier3(pars::get(pars::outerPredefinedSellmeier3, args...)));
			}

			pars::set(fOuterPictureColor,
				pars::pictureColor,
				args...);
			pars::set(fInnerPictureColor,
				pars::pictureColor,
				args...);

			pars::set(fInnerPictureColor,
				pars::innerPictureColor,
				args...);

			pars::set(fOuterPictureColor,
				pars::outerPictureColor,
				args...);

			pars::set(fSpectrum,
				pars::spectrum,
				args...);

			pars::set(fBrightness,
				pars::brightness,
				args...);

		}

		virtual void dummy() {}

		// by default surface is non-trans and fully relfective
		Texture fIn2OutReflect = 1;
		Texture fOut2InReflect = 1;
		Texture fIn2OutTrans = 0;
		Texture fOut2InTrans = 0;

		ReflectType fInnerReflectType = ReflectType::Diffuse;
		ReflectType fOuterReflectType = ReflectType::Diffuse;

		// for mirror refraction
		RefractiveIndex fIndexInner{ 1. };
		RefractiveIndex fIndexOuter{ 1. };

		// for lighting
		// only diffuse model
		Real fBrightness = 0.;
		std::shared_ptr<Spectrum> fSpectrum;

		// for picture
		Color fInnerPictureColor = Color::white(1.);
		Color fOuterPictureColor = Color::white(1.);
	};


	inline void SurfaceProperties::setBrightness(Real amp)
	{
		fBrightness = amp;
	}




	inline void SurfaceProperties::setPictureColor(Color c)
	{
		fInnerPictureColor = c;
		fOuterPictureColor = c;
	}

	inline void SurfaceProperties::setInnerPictureColor(Color c)
	{
		fInnerPictureColor = c;
	}

	inline void SurfaceProperties::setOuterPictureColor(Color c)
	{
		fOuterPictureColor = c;
	}

	inline void SurfaceProperties::setPictureAlpha(Real a)
	{
		fInnerPictureColor.A() = a;
		fOuterPictureColor.A() = a;
	}
	inline void SurfaceProperties::setInnerPictureAlpha(Real a)
	{
		fInnerPictureColor.A() = a;
	}

	inline void SurfaceProperties::setInnerReflectType(ReflectType type)
	{
		fInnerReflectType = type;
	}

	inline void SurfaceProperties::setOuterReflectType(ReflectType type)
	{
		fOuterReflectType = type;
	}

	inline void SurfaceProperties::setReflectType(ReflectType type)
	{
		setInnerReflectType(type);
		setOuterReflectType(type);
	}


	inline void SurfaceProperties::setInnerIndex(Real index)
	{
		fIndexInner.set(index);
	}

	inline void SurfaceProperties::setOuterIndex(Real index)
	{
		fIndexOuter.set(index);
	}

	inline void SurfaceProperties::setInnerIndex(std::shared_ptr<UnitaryFunction> index)
	{
		fIndexInner.set(index);
	}
	inline void SurfaceProperties::setOuterIndex(std::shared_ptr<UnitaryFunction> index)
	{
		fIndexOuter.set(index);
	}

	inline void SurfaceProperties::setTrans(Real v)
	{
		setInnerTrans(v);
		setOuterTrans(v);
	}

	inline void SurfaceProperties::setInnerTrans(Real v)
	{
		fIn2OutTrans.setRatio(v);
	}

	inline void SurfaceProperties::setOuterTrans(Real v)
	{
		fOut2InTrans.setRatio(v);
	}

	inline void SurfaceProperties::setReflect(Real v)
	{
		setInnerReflect(v);
		setOuterReflect(v);
	}

	inline void SurfaceProperties::setInnerReflect(Real v)
	{
		fIn2OutReflect.setRatio(v);
	}

	inline void SurfaceProperties::setOuterReflect(Real v)
	{
		fOut2InReflect.setRatio(v);
	}




	inline void SurfaceProperties::setTrans(std::shared_ptr<TextureInterface> v)
	{
		setInnerTrans(v);
		setOuterTrans(std::move(v));
	}

	inline void SurfaceProperties::setInnerTrans(std::shared_ptr<TextureInterface> v)
	{
		fIn2OutTrans.setFunction(std::move(v));
	}

	inline void SurfaceProperties::setOuterTrans(std::shared_ptr<TextureInterface> v)
	{
		fOut2InTrans.setFunction(std::move(v));
	}

	inline void SurfaceProperties::setReflect(std::shared_ptr<TextureInterface> v)
	{
		setInnerReflect(v);
		setOuterReflect(std::move(v));
	}

	inline void SurfaceProperties::setInnerReflect(std::shared_ptr<TextureInterface> v)
	{
		fIn2OutReflect.setFunction(std::move(v));
	}

	inline void SurfaceProperties::setOuterReflect(std::shared_ptr<TextureInterface> v)
	{
		fOut2InReflect.setFunction(std::move(v));
	}

}
#endif
