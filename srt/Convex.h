#pragma once

#include "Vec3.h"
#include "Surface.h"

namespace srt {

	struct Convex : Surface
	{
		void addSurface(std::shared_ptr<Surface> surf);
		bool isInner(Vec3 const& p) const override;
		void process(Ray const& ray,
			ProcessHandler& handler) const override;

	private:
		std::vector<std::shared_ptr<Surface>> fSurfaces;
		// the faces as planes, filled only while every face is a PlaneSurface
		std::vector<struct PlaneSurface const*> fPlanes;
		bool fAllPlanes = true;
	};

	std::shared_ptr<Convex> convex(std::initializer_list<std::shared_ptr<Surface>> surfaces);

}
