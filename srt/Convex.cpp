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
		Real smin = kInfity;
		Surface* surfmin = nullptr;
		for (Surface* surf : unwrap(fSurfaces)) {
			Hit h;
			if (surf->intersect(ray, smin, h)) {

				Vec3 p = ray.fO + h.t * ray.fD;
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
					smin = h.t;
					surfmin = surf;
				}
			}
		}
		return surfmin;
	}

	bool Convex::intersect(Ray const& ray, Real tMax, Hit& hit) const
	{
		Surface const* face;
		bool done = false;
		if (fAllPlanes) {
			face = minsPlane(ray, fPlanes, done);
		}
		if (!done) {
			face = minsSurface(ray, fSurfaces);
		}
		if (!face || !face->intersect(ray, tMax, hit)) {
			return false;
		}
		if (!hit.sub) {
			hit.sub = face;
		}
		return true;
	}

	void Convex::shade(Ray const& ray, Hit const& hit, TracingHandler& out) const
	{
		// only reached if the hit was not passed to its face (Hit::sub)
		if (hit.sub) {
			hit.sub->shade(ray, hit, out);
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
