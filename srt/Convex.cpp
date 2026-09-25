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

	// For a body bounded only by unbounded planes: along the ray, each plane
	// is either entered (f(x) = P.x + R turns negative) or left, so the ray
	// is inside the body over [latest entry, earliest exit]. This takes one
	// test per face instead of checking every hit against all other faces.
	// Sets done = false if a face has a bound and the general search is needed.
	static PlaneSurface const* minsPlane(Ray const& ray,
		std::vector<PlaneSurface const*> const& planes, bool& done)
	{
		done = true;
		Real enter = -kInfity, exit = kInfity;
		PlaneSurface const* enterFace = nullptr;
		PlaneSurface const* exitFace = nullptr;
		for (PlaneSurface const* plane : planes) {
			if (plane->getBound()) {
				done = false;
				return nullptr;
			}
			Real a = dot(plane->fP, ray.fD);
			Real b = dot(plane->fP, ray.fO) + plane->fR;
			if (a == 0) {
				if (b >= 0) {
					return nullptr;	// parallel to the face, outside it
				}
				continue;
			}
			Real s = -b / a;
			if (a < 0) {
				if (s > enter) {
					enter = s;
					enterFace = plane;
				}
			} else {
				if (s < exit) {
					exit = s;
					exitFace = plane;
				}
			}
		}
		if (!(enter < exit)) {
			// misses, or only touches the body where enter and exit meet: a
			// tangent ray, whose hit depends on rounding, so leave it to the
			// general search to match its choice exactly
			done = !(enter <= exit + 1E-9 * (1 + fabs(exit)));
			return nullptr;
		}
		if (exit - enter <= 1E-9 * (1 + fabs(enter))) {
			done = false;
			return nullptr;
		}
		if (enter > gSmin) {
			return enterFace;
		}
		if (exit > gSmin) {
			return exitFace;
		}
		return nullptr;
	}

	void Convex::process(Ray const& ray,
		ProcessHandler& handler) const
	{
		if (fAllPlanes) {
			bool done;
			PlaneSurface const* face = minsPlane(ray, fPlanes, done);
			if (done) {
				if (face)
					face->process(ray, handler);
				return;
			}
		}
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
		return c;
	}



}
