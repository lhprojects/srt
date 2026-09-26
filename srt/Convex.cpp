#include "Convex.h"
#include "Surfaces.h"

namespace srt {

	void Convex::addSurface(std::shared_ptr<Surface> surf)
	{
		auto plane = dynamic_cast<PlaneSurface const*>(surf.get());
		if (plane && fAllPlanes) {
			fPlanes.push_back(plane);
		} else {
			fAllPlanes = false;
			fPlanes.clear();
		}
		fSurfaces.push_back(std::move(surf));
	}

	bool Convex::isInner(Vec3 const& p) const
	{
		for (auto& surf : fSurfaces) {
			if (!surf->isInner(p)) return false;
		}
		return true;
	}

	Surface *minsSurface(Ray const &ray,
		std::vector<std::shared_ptr<Surface>> const& fSurfaces) {
		DistanceHandler tdh;
		Real smin = kInfity;
		Surface* surfmin = nullptr;
		for (Surface* surf : unwrap(fSurfaces)) {
			tdh.fDistance = kInfity;
			surf->process(ray, tdh);
			if (tdh.fDistance < smin) {

				Vec3 p = ray.fO + tdh.fDistance * ray.fD;
				bool innner = true;
				for (Surface* surf2 : unwrap(fSurfaces)) {
					if (surf2 != surf) {
						if (!surf2->isInner(p)) {
							innner = false;
							break;
						}
					}
				}
				if (innner) {
					smin = tdh.fDistance;
					surfmin = surf;
				}
			}
		}
		return surfmin;
	}

	void Convex::process(Ray const& ray,
		ProcessHandler& handler) const
	{
		if (fAllPlanes) {
			bool done;
			PlaneSurface const* face = minsPlane(ray, fPlanes, done);
			if (done) {
				if (face)
					face->PlaneSurface::process(ray, handler);
				return;
			}
		}
		Surface* surfmin = minsSurface(ray, fSurfaces);
		if (surfmin)
			surfmin->process(ray, handler);
	}

	void Convex::distanceGeneral(Ray const& ray, Real& s, bool& in2out) const
	{
		s = kInfity;
		Surface* surfmin = minsSurface(ray, fSurfaces);
		if (surfmin) {
			DistanceHandler handler;
			surfmin->process(ray, handler);
			s = handler.fDistance;
			in2out = handler.fIn2out;
		}
	}

	std::shared_ptr<Convex> convex(std::initializer_list<std::shared_ptr<Surface>> surfaces)
	{
		auto c = std::make_shared<Convex>();
		for (auto& s : surfaces) {
			c->addSurface(s);
		}
		return c;
	}



}
