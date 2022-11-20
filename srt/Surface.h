#pragma once

#include "Device.h"
#include "Vec3.h"
#include "Bound.h"
#include <vector>
#include <string>
#include <memory>
#include <assert.h>

namespace srt {

	template<class Iter>
	struct UnwrapIter {

		UnwrapIter(Iter iter) :iter(iter) {}

		auto operator*()
		{
			return iter->get();
		}
		auto&& operator++()
		{
			return ++iter;
		}
		auto&& operator--()
		{
			return --iter;
		}
		bool operator!=(UnwrapIter const& r) const
		{
			return iter != r.iter;
		}

		bool operator==(UnwrapIter const& r) const
		{
			return iter == r.iter;
		}
		Iter iter;
	};

	template<class R>
	struct Unwrap {

		Unwrap(R&& r) :r(std::forward<R>(r)) {}
		auto begin()
		{
			using Iter = std::remove_cvref_t<decltype(r.begin())>;
			return UnwrapIter<Iter>(r.begin());
		}
		auto end()
		{
			using Iter = std::remove_cvref_t<decltype(r.end())>;
			return UnwrapIter<Iter>(r.end());
		}
		R&& r;
	};

	template<class R>
	auto unwrap(R&& r)
	{
		return Unwrap<R>(std::forward<R>(r));
	}


	// A surface devides the space into two parts
	struct Surface : Device, SurfaceProperties
	{

		Bound* getBound();
		Bound const* getBound() const;
		void setBound(std::shared_ptr<Bound> b);

		// is p at the inner side of the surface ?
		virtual bool isInner(Vec3 const& p) const = 0;

		using Pars = pars::MergePars<Device::Pars,
			SurfaceProperties::Pars,
			pars::Pars<pars::bound_>>;


		void set(pars::argument auto const &... args)
		{
			pars::check<Pars>(args...);
			set(pars::uncheck, args...);
		}

		void set(pars::uncheck_t,
			pars::argument auto const &... args)
		{
			pars::set(fBound, pars::bound, args...);
			Device::set(pars::uncheck, args...);
			SurfaceProperties::set(pars::uncheck, args...);
		}

	private:
		std::shared_ptr<Bound> fBound;
	};
	std::shared_ptr<Bound> asBound(std::shared_ptr<Surface>);

}

// implementation
namespace srt {
	inline Bound* Surface::getBound()
	{
		return fBound.get();
	}

	inline Bound const* Surface::getBound() const
	{
		return fBound.get();
	}

	inline void Surface::setBound(std::shared_ptr<Bound> b)
	{
		fBound = std::move(b);
	}

}

