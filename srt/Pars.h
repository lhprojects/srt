#pragma once

#include "Real.h"
#include "Vec3.h"
#include "Color.h"
#include "ReflectType.h"
#include "Align.h"

#include <stdint.h>
#include <memory>
#include <utility>
#include <string>
#include <type_traits>

namespace srt {


	enum class PredefinedSellmeier3 {
		BK7,
	};

	enum class ShapeType {
		Plane,
		Shpere,
		Parabola,
		Tube,
	};

	struct Bound;
	struct Spectrum;

	namespace pars {

		struct arg_tag {};
		struct uncheck_t {};
		constexpr uncheck_t uncheck;

		template<class ParTag, class Value>
		struct arg {

			template<class V>
			arg(arg_tag, V&& v) : value(std::forward<V>(v))
			{
			}

			Value value;
			using Par = ParTag;
		};

		template<class Arg>
		struct is_argument {
			static const bool value = false;
		};

		template<class Par, class ArgT>
		struct is_argument< arg<Par, ArgT> > {
			static const bool value = true;
		};

		template<class Arg>
		concept argument = is_argument<std::remove_cvref_t<Arg> >::value;

		template<class ParTag, class ParV >
		struct par {

			par& operator=(par const& v) = delete;
			par& operator=(par&& v) = delete;

			template<class V>
			requires std::is_convertible_v<V&&, ParV>
			arg<ParTag, ParV> operator=(V&& v) const
			{
				return arg<Par, ParV>(arg_tag(), std::forward<V>(v));
			}

			using Par = ParTag;
		};


		template<class Par>
		struct is_parameter {
			static const bool value = false;
		};

		template<class Par, class ArgT>
		struct is_parameter< par<Par, ArgT> > {
			static const bool value = true;
		};

		template<class Par>
		concept parameter = is_parameter<std::remove_cvref_t<Par> >::value;


		template<class... Par>
		class Pars {};

		template<class Arg, class Par, class T>
		constexpr bool has1(par<Par, T> const& p)
		{
			return std::is_same_v<Arg::Par, Par>;
		}

		//argument has wanted parameter?
		template<argument... Args, class Par, class T>
		constexpr bool has2(par<Par, T> const& p)
		{
			return (has1<Args>(p) || ...);
		}

		//arguments has wanted parameter?
		template<argument... Args, class Par, class T>
		constexpr bool has(par<Par, T> const &p)
		{
			return has2<std::remove_cvref_t<Args>...>(p);
		}

		template<class Par, class T, class Par0, class Arg0T, argument... Args>
		auto&& get(par<Par, T> const& p,
			arg<Par0, Arg0T>& arg0,
			Args &&... args)
		{
			if constexpr (std::is_same_v<Par, Par0>) {
				return arg0.value;
			}
			else {
				return get(p, std::forward<Args>(args)...);
			}
		}

		template<class Par, class T, class Par0, class Arg0T, argument... Args>
		auto&& get(par<Par, T> const& p,
			arg<Par0, Arg0T> const& arg0,
			Args &&... args)
		{
			if constexpr (std::is_same_v<Par, Par0>) {
				return arg0.value;
			}
			else {
				return get(p, std::forward<Args>(args)...);
			}
		}

		template<class Par, class T, class Def, argument... Args>
		auto&& get_default(par<Par, T> const &p,
			Def&& def, Args && ... args)
		{
			if constexpr (has<Args...>(par<Par, T>())) {
				return get(par<Par, T>(), std::forward<Args>(args)...);
			}
			else {
				return std::forward<Def>(def);
			}
		}

		template<class Par, class T, argument... Args>
		void set_default(T& v, par<Par, T> const& p,
			T def, Args const& ... args)
		{
			if (has(p, args...)) {
				v = get(p, args...);
			}
			else {
				v = def;
			}
		}

		template<class Par, class T, class... Args>
		void set(T& v, par<Par, T> const& p, Args const& ... args)
		{
			if constexpr (has<Args...>(par<Par, T>())) {
				v = get(p, args...);
			}
		}


		// check the argument in parameters
		template<class... Pars_, class ArgPar, class ArgT>
		void check_1arg_in_pars(Pars<Pars_...>, arg<ArgPar, ArgT> const& arg)
		{
			static_assert(((std::is_same_v<ArgPar, Pars_>) || ...), "Arg must be one of Parameters");
		}

		// check arguments in parameters
		template<class ParsPack, argument... Args>
		void check(argument auto const&... args)
		{
			(check_1arg_in_pars(ParsPack(), args), ...);
		}


		template<class par1, class par2>
		struct Merge2ParsImpl;

		template<class... Pars1, class... Pars2>
		struct Merge2ParsImpl<Pars<Pars1...>, Pars<Pars2...>> {
			using type = Pars<Pars1..., Pars2...>;
		};

		template<class... Rem>
		struct MergeParsImpl;

		template<class ParPack1, class ParPack2, class... Rem>
		struct MergeParsImpl<ParPack1, ParPack2, Rem...> {
			using type = typename MergeParsImpl<
				typename Merge2ParsImpl<ParPack1, ParPack2>::type,
				Rem...>::type;
		};

		template<class ParPack1>
		struct MergeParsImpl<ParPack1> {
			using type = ParPack1;
		};

		template<class... ParPacks>
		using MergePars = typename MergeParsImpl<ParPacks...>::type;
		
	}

	//#define CONCEPT_MODULE 
	//#include "Pars_.h"

	namespace pars {

		struct name_;
		constexpr par<name_, std::string> name{};
		struct radius_;
		constexpr par<radius_, Real> radius{};
		struct color_;
		constexpr par<color_, srt::Color> color{};
		struct fontSize_;
		constexpr par<fontSize_, Real> fontSize{};
		struct verticalAlign_; constexpr par<verticalAlign_, VerticalAlign> verticalAlign{};
		struct horizentalAlign_; constexpr par<horizentalAlign_, HorizentalAlign> horizentalAlign{};
		struct high_; constexpr par<high_, int> high{};
		struct width_; constexpr par<width_, int> width{};
		struct origin_;
		constexpr par<origin_, Vec3> origin{};
		struct direction_;
		constexpr par<direction_, Vec3> direction{};
		struct n1_;
		constexpr par<n1_, Vec3> n1{};
		struct n1Min_;
		constexpr par<n1Min_, Real> n1Min{};
		struct n1Max_;
		constexpr par<n1Max_, Real> n1Max{};
		struct n2_;
		constexpr par<n2_, Vec3> n2{};
		struct n2Min_;
		constexpr par<n2Min_, Real> n2Min{};
		struct n2Max_;
		constexpr par<n2Max_, Real> n2Max{};
		struct screenSize_; constexpr par<screenSize_, Real> screenSize{};
		struct fieldOfView_; constexpr par<fieldOfView_, Real> fieldOfView{};
		struct fieldOfView1_; constexpr par<fieldOfView1_, Real> fieldOfView1{};
		struct fieldOfView2_; constexpr par<fieldOfView2_, Real> fieldOfView2{};

		struct antiAliasLevel_;
		constexpr par<antiAliasLevel_, int> antiAliasLevel{};
		struct samplePerPixel_;
		constexpr par<samplePerPixel_, int> samplePerPixel{};
		struct lightOrigin_;
		constexpr par<lightOrigin_, Vec3> lightOrigin{};

		// it is general said depth of field is determined by 
		// apertureDiameter (= f / f-number) and focalDistance.
		// (circle of confusion, not relevant here)
		// assume point-subject-at-u0 is exactly a point-image-at-v0
		// then:
		// radius-of-image-of-point-subject-at-u'/v0 = D/u0(1-u0/u')
		// where u0 is the focalDistance and D is the aperture Diameter
		// the character of blur of back/fore-ground is determined by the D and u0
		// alpha = D DOF/u0^2
		// D = alpha*u0^2/DOF, alpha is the angular resolution

		struct lookAt_; constexpr par<lookAt_, Vec3> lookAt{};
		// distacne-to-subject, not focal length = f
		struct focalDistance_; constexpr par<focalDistance_, Real> focalDistance;
		struct apertureDiameter_; constexpr par<apertureDiameter_, Real> apertureDiameter;
		struct gray_; constexpr par<gray_, bool> gray{};

	
		struct outerReflectType_; constexpr par<outerReflectType_, ReflectType> outerReflectType{};
		struct innerReflectType_; constexpr par<innerReflectType_, ReflectType> innerReflectType{};
		struct innerReflectRatio_; constexpr par<innerReflectRatio_, Real> innerReflectRatio{};
		struct outerReflectRatio_; constexpr par<outerReflectRatio_, Real> outerReflectRatio{};
		struct reflectType_; constexpr par<reflectType_, ReflectType> reflectType{};
		struct reflectRatio_; constexpr par<reflectRatio_, Real> reflectRatio{};
		struct refractRatio_; constexpr par<refractRatio_, Real> refractRatio{};
		struct in2OutRefractRatio_; constexpr par<in2OutRefractRatio_, Real> in2OutRefractRatio{};
		struct out2InRefractRatio_; constexpr par<out2InRefractRatio_, Real> out2InRefractRatio{};
		struct innerIndex_; constexpr par<innerIndex_, Real> innerIndex{};
		struct innerPredefinedSellmeier3_; constexpr par<innerPredefinedSellmeier3_, PredefinedSellmeier3> innerPredefinedSellmeier3{};
		struct outerIndex_; constexpr par<outerIndex_, Real> outerIndex{};

		struct outerPredefinedSellmeier3_; constexpr par<outerPredefinedSellmeier3_, PredefinedSellmeier3> outerPredefinedSellmeier3{};
		struct innerPictureColor_; constexpr par<innerPictureColor_, Color> innerPictureColor{};
		struct outerPictureColor_; constexpr par<outerPictureColor_, Color> outerPictureColor{};
		struct pictureColor_; constexpr par<pictureColor_, Color> pictureColor{};
		struct shape_; constexpr par<shape_, ShapeType> shape{};
		struct recordIn2Out_; constexpr par<recordIn2Out_, bool> recordIn2Out{};
		struct recordOut2In_; constexpr par<recordOut2In_, bool> recordOut2In{};
		struct record_; constexpr par<record_, bool> record{};
		struct bound_; constexpr par<bound_, std::shared_ptr<Bound>> bound{};
		// multiple-threading
		struct mult_; constexpr par<mult_, bool> mult{};
		struct stdoutProgress_; constexpr par< stdoutProgress_, bool> stdoutProgress{};
		struct x0_; constexpr par<x0_, Real> x0{};
		struct x1_; constexpr par<x1_, Real> x1{};
		struct y0_; constexpr par<y0_, Real> y0{};
		struct y1_; constexpr par<y1_, Real> y1{};
		struct z0_; constexpr par<z0_, Real> z0{};
		struct z1_; constexpr par<z1_, Real> z1{};
		struct amp_; constexpr par<amp_, Real> amp{};
		struct spectrum_; constexpr par<spectrum_, std::shared_ptr<Spectrum>> spectrum{};
		struct brightness_; constexpr par<brightness_, Real> brightness{};
	}


}
