#include "Convex.h"
#include "Surfaces.h"

namespace srt {

	void Convex::addSurface(std::shared_ptr<Surface> surf)
	{
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
		Surface* surfmin = minsSurface(ray, fSurfaces);
		if (surfmin)
			surfmin->process(ray, handler);
	}

	std::shared_ptr<Convex> convex(std::initializer_list<std::shared_ptr<Surface>> surfaces)
	{
		auto c = std::make_shared<Convex>();
		for (auto& s : surfaces) {
			c->addSurface(s);
		}
		return std::move(c);
	}



}
