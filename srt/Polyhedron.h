#pragma once

#include <memory>
#include <vector>
#include <utility>
#include <initializer_list>
#include "Vec3.h"
#include "Ray.h"
#include "Pars.h"
#include "Device.h"
#include "SurfaceProperties.h"
#include "Engine.h"

namespace srt {

	// A face of a polyhedron: the plane dot(N, x) + R = 0, N the unit normal
	// pointing out of the body, and the face's own material.
	struct PolyhedronFace : SurfaceProperties
	{
		Vec3 N{};
		Real R = 0;

		// the plane through origin; direction points out of the body
		void setPlane(Vec3 const& origin, Vec3 const& direction);
		// squares of side w with ratio 1 and 0.5, as PlaneSurface::setGridTexture
		void setGridTexture(Real w);
	};

	// What both polyhedra share: the faces, their materials and the device
	// interface. The intersection itself is the subclass's.
	struct PolyhedronBase : Device
	{
		static constexpr auto pars_ = Device::pars_ | SurfaceProperties::pars_;

		PolyhedronFace& face(size_t i) { return fFaces[i]; }
		PolyhedronFace const& face(size_t i) const { return fFaces[i]; }
		size_t faceCount() const { return fFaces.size(); }

		// sets the name, and the material of every face
		void set(pars::argument auto const &... args)
		{
			pars::check(pars_, args...);
			Device::set(pars::uncheck, args...);
			for (auto& f : fFaces) {
				f.set(pars::uncheck, args...);
			}
		}

	protected:
		// hit on face i at distance s: fills handler as a surface would
		void report(Ray const& ray, int i, Real s, ProcessHandler& handler) const;

		std::vector<PolyhedronFace> fFaces;
	};

	// A convex polyhedron: the part of space inside the planes of all its
	// faces. Along a ray each face is either entered or left, so the ray is
	// inside over [latest entry, earliest exit]: one test per face.
	struct ConvexPolyhedron : PolyhedronBase
	{
		ConvexPolyhedron() { fKind = Kind::ConvexPolyhedron; }

		// adds the face on the plane through origin, direction pointing out
		PolyhedronFace& addFace(Vec3 const& origin, Vec3 const& direction);
		// moves face i onto another plane
		void setFace(size_t i, Vec3 const& origin, Vec3 const& direction);

		void process(Ray const& ray, ProcessHandler& handler) const override;
		// the distance pass of process(): s = kInfity on a miss
		void distance(Ray const& ray, Real& s, bool& in2out) const;

	private:
		// the face where the ray enters or leaves the body ahead of it, -1 if
		// none; s its distance, in2out whether the ray leaves there
		int hitFace(Ray const& ray, Real& s, bool& in2out) const;
	};

	// the convex polyhedron with these faces: (origin, direction) as addFace
	std::shared_ptr<ConvexPolyhedron> convexPolyhedron(
		std::initializer_list<std::pair<Vec3, Vec3>> planes);
	// the convex hull of the points (at least 4, not all on one plane)
	std::shared_ptr<ConvexPolyhedron> convexHull(std::vector<Vec3> const& points);

	// A polyhedron of any shape, convex or not, given by its vertices and its
	// faces. It must be closed, and each face flat. Each face is tested: the
	// nearest hit of the ray on a face plane that lies inside that face.
	struct Polyhedron : PolyhedronBase
	{
		Polyhedron() { fKind = Kind::Polyhedron; }

		// faces[i]: the indices of the vertices of face i, in order around it,
		// counterclockwise seen from outside (or all faces clockwise: the
		// orientation is taken from the sign of the volume)
		void setMesh(std::vector<Vec3> vertices,
			std::vector<std::vector<int>> faces);
		// moves vertex i; the faces around it must stay flat
		void setVertex(size_t i, Vec3 const& p);
		Vec3 const& vertex(size_t i) const { return fVertices[i]; }

		void process(Ray const& ray, ProcessHandler& handler) const override;
		// the distance pass of process(): s = kInfity on a miss
		void distance(Ray const& ray, Real& s, bool& in2out) const;

	private:
		// the planes and outlines of the faces, from the vertices
		void update();
		int hitFace(Ray const& ray, Real& s, bool& in2out) const;

		std::vector<Vec3> fVertices;
		std::vector<std::vector<int>> fIndices;
		// face i projected onto the coordinate plane dropping axis fDrop[i]
		std::vector<int> fDrop;
		std::vector<std::vector<Real>> fU, fV;
	};

	std::shared_ptr<Polyhedron> polyhedron(std::vector<Vec3> vertices,
		std::vector<std::vector<int>> faces);

	inline int ConvexPolyhedron::hitFace(Ray const& ray, Real& s,
		bool& in2out) const
	{
		Real enter = -kInfity, exit = kInfity;
		int enterFace = -1, exitFace = -1;
		for (size_t i = 0; i < fFaces.size(); ++i) {
			PolyhedronFace const& f = fFaces[i];
			Real a = dot(f.N, ray.fD);
			Real b = dot(f.N, ray.fO) + f.R;
			if (a == 0) {
				if (b >= 0) {
					return -1;	// parallel to the face, outside it
				}
				continue;
			}
			Real t = -b / a;
			if (a < 0) {
				if (t > enter) {
					enter = t;
					enterFace = (int)i;
				}
			} else {
				if (t < exit) {
					exit = t;
					exitFace = (int)i;
				}
			}
		}
		if (!(enter < exit)) {
			return -1;	// misses the body
		}
		if (enter > gSmin) {
			s = enter;
			in2out = false;
			return enterFace;
		}
		if (exit > gSmin) {
			s = exit;
			in2out = true;
			return exitFace;
		}
		return -1;
	}

	inline void ConvexPolyhedron::distance(Ray const& ray, Real& s,
		bool& in2out) const
	{
		if (hitFace(ray, s, in2out) < 0) {
			s = kInfity;
		}
	}

}
