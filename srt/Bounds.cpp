#include "Bounds.h"
namespace srt {


	void AllBound::addBound(std::shared_ptr<Bound> b)
	{
		fBounds.push_back(std::move(b));
	}

	bool AllBound::onInBound(Vec3 const& p) const
	{
		for (auto& b : fBounds) {
			if (!b->inBound(p)) return false;
		}
		return true;
	}

	void AnyBound::addBound(std::shared_ptr<Bound> b)
	{
		fBounds.push_back(std::move(b));
	}

	bool AnyBound::onInBound(Vec3 const& p) const
	{
		for (auto& b : fBounds) {
			if (b->inBound(p)) return true;
		}
		return false;
	}

	InverseBound::InverseBound(std::shared_ptr<Bound> b) :
		fBound(std::move(b)) {
	}

	bool InverseBound::onInBound(Vec3 const& p) const {
		return !fBound || !fBound->inBound(p);
	}

	void InverseBound::setBound(std::shared_ptr<Bound> b) {
		fBound = b;
	}

	std::shared_ptr<AllBound> all(std::initializer_list<std::shared_ptr<Bound>> bounds)
	{
		auto a = std::make_shared<AllBound>();
		for (auto& b : bounds) {
			a->addBound(b);
		}
		return std::move(a);
	}

	std::shared_ptr<AnyBound> any(std::initializer_list<std::shared_ptr<Bound>> bounds)
	{
		auto a = std::make_shared<AnyBound>();
		for (auto& b : bounds) {
			a->addBound(b);
		}
		return std::move(a);
	}

	std::shared_ptr<InverseBound> inverse(std::shared_ptr<Bound> bound)
	{
		return std::make_shared<InverseBound>(std::move(bound));
	}

	SurfaceBound::SurfaceBound(std::shared_ptr<Surface> ss) : s(std::move(ss)) {
	}

	bool SurfaceBound::onInBound(Vec3 const& p) const {
		return s->isInner(p);
	}

	std::shared_ptr<Bound> asBound(std::shared_ptr<Surface> s) {
		return std::make_shared< SurfaceBound>(std::move(s));
	}

}
