#pragma once

#include "Vec3.h"
#include "Surface.h"
#include "Surfaces.h"

namespace srt {

	struct Convex : Surface
	{
		Convex() { fKind = Kind::Convex; }

		void addSurface(std::shared_ptr<Surface> surf);
		bool isInner(Vec3 const& p) const override;
		void process(Ray const& ray,
			ProcessHandler& handler) const override;

		// the distance pass of process(): s = kInfity on a miss
		inline void distance(Ray const& ray, Real& s, bool& in2out) const;

	private:
		// distance() for a body not made of unbounded planes only
		void distanceGeneral(Ray const& ray, Real& s, bool& in2out) const;

		std::vector<std::shared_ptr<Surface>> fSurfaces;
		// the faces as planes, filled only while every face is a PlaneSurface
		std::vector<struct PlaneSurface const*> fPlanes;
		bool fAllPlanes = true;
	};

	std::shared_ptr<Convex> convex(std::initializer_list<std::shared_ptr<Surface>> surfaces);

	// For a body bounded only by unbounded planes: along the ray, each plane
	// is either entered (f(x) = P.x + R turns negative) or left, so the ray
	// is inside the body over [latest entry, earliest exit]. This takes one
	// test per face instead of checking every hit against all other faces.
	// Sets done = false if a face has a bound and the general search is needed.
	inline PlaneSurface const* minsPlane(Ray const& ray,
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

	inline void Convex::distance(Ray const& ray, Real& s, bool& in2out) const
	{
		if (fAllPlanes) {
			bool done;
			PlaneSurface const* face = minsPlane(ray, fPlanes, done);
			if (done) {
				if (face) {
					face->distance(ray, s, in2out);
				} else {
					s = kInfity;
				}
				return;
			}
		}
		distanceGeneral(ray, s, in2out);
	}

}
