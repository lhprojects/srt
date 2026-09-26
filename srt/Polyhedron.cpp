#include "Polyhedron.h"
#include <cmath>
#include <stdexcept>

namespace srt {

	void PolyhedronFace::setPlane(Vec3 const& origin, Vec3 const& direction)
	{
		N = normalize(direction);
		R = -dot(N, origin);
	}

	void PolyhedronFace::setGridTexture(Real w)
	{
		fIn2OutReflect.setGrid(N, w);
		fOut2InReflect.setGrid(N, w);
	}

	void PolyhedronBase::report(Ray const& ray, int i, Real s,
		ProcessHandler& handler) const
	{
		PolyhedronFace const& f = fFaces[i];
		bool in2out = dot(f.N, ray.fD) > 0;
		if (handler.fType == HandlerType::Distance) {
			static_cast<DistanceHandler&>(handler).distance(s, in2out);
		} else if (handler.fType == HandlerType::Tracing) {
			static_cast<TracingHandler&>(handler).hitSurface(
				ray.fO + ray.fD * s, f.N, in2out, &f, this);
		}
	}

	// ---- ConvexPolyhedron

	PolyhedronFace& ConvexPolyhedron::addFace(Vec3 const& origin,
		Vec3 const& direction)
	{
		fFaces.emplace_back();
		fFaces.back().setPlane(origin, direction);
		return fFaces.back();
	}

	void ConvexPolyhedron::setFace(size_t i, Vec3 const& origin,
		Vec3 const& direction)
	{
		fFaces[i].setPlane(origin, direction);
	}

	void ConvexPolyhedron::process(Ray const& ray, ProcessHandler& handler) const
	{
		Real s;
		bool in2out;
		int i = hitFace(ray, s, in2out);
		if (i >= 0) {
			report(ray, i, s, handler);
		}
	}

	std::shared_ptr<ConvexPolyhedron> convexPolyhedron(
		std::initializer_list<std::pair<Vec3, Vec3>> planes)
	{
		auto p = std::make_shared<ConvexPolyhedron>();
		for (auto const& [origin, direction] : planes) {
			p->addFace(origin, direction);
		}
		return p;
	}

	std::shared_ptr<ConvexPolyhedron> convexHull(std::vector<Vec3> const& points)
	{
		// a face of the hull is a plane through three of the points with all
		// the points on one side of it; few points, so try every triple
		Real size = 0;
		for (auto const& p : points) {
			size = std::max(size, std::sqrt(norm2(p - points[0])));
		}
		Real const eps = 1E-9 * (1 + size);

		auto p = std::make_shared<ConvexPolyhedron>();
		size_t n = points.size();
		for (size_t i = 0; i < n; ++i)
			for (size_t j = i + 1; j < n; ++j)
				for (size_t k = j + 1; k < n; ++k) {
					Vec3 N = cross(points[j] - points[i], points[k] - points[i]);
					if (norm2(N) <= eps * eps * eps * eps) {
						continue;	// the three are on a line
					}
					N = normalize(N);
					Real R = -dot(N, points[i]);
					bool below = true, above = true;
					for (auto const& q : points) {
						Real f = dot(N, q) + R;
						below = below && f <= eps;
						above = above && f >= -eps;
					}
					if (!below && !above) {
						continue;
					}
					if (!below) {
						// the points are all above: turn N out of the body
						N = -N;
						R = -R;
					}
					bool seen = false;
					for (size_t m = 0; m < p->faceCount(); ++m) {
						PolyhedronFace const& f = p->face(m);
						if (dot(f.N, N) > 1 - 1E-12 && std::fabs(f.R - R) <= eps) {
							seen = true;
							break;
						}
					}
					if (!seen) {
						p->addFace(-R * N, N);
					}
				}
		if (p->faceCount() < 4) {
			throw std::invalid_argument("convexHull: the points span no volume");
		}
		return p;
	}

	// ---- Polyhedron

	void Polyhedron::setMesh(std::vector<Vec3> vertices,
		std::vector<std::vector<int>> faces)
	{
		fVertices = std::move(vertices);
		fIndices = std::move(faces);
		fFaces.resize(fIndices.size());
		update();
	}

	void Polyhedron::setVertex(size_t i, Vec3 const& p)
	{
		fVertices[i] = p;
		update();
	}

	void Polyhedron::update()
	{
		size_t nf = fIndices.size();
		fDrop.resize(nf);
		fU.resize(nf);
		fV.resize(nf);

		// the normal of each face by Newell's method: its length is twice the
		// area, its direction follows the order of the vertices
		Real volume = 0;
		for (size_t i = 0; i < nf; ++i) {
			auto const& idx = fIndices[i];
			Vec3 A{};
			Vec3 c{};
			for (size_t k = 0; k < idx.size(); ++k) {
				Vec3 const& p = fVertices[idx[k]];
				Vec3 const& q = fVertices[idx[(k + 1) % idx.size()]];
				A.fX += (p.fY - q.fY) * (p.fZ + q.fZ);
				A.fY += (p.fZ - q.fZ) * (p.fX + q.fX);
				A.fZ += (p.fX - q.fX) * (p.fY + q.fY);
				c += p;
			}
			c *= 1. / idx.size();
			volume += dot(c, A);
			fFaces[i].N = normalize(A);
			fFaces[i].R = -dot(fFaces[i].N, c);
		}
		if (volume < 0) {
			// the faces were given clockwise: turn the normals out
			for (auto& f : fFaces) {
				f.N = -f.N;
				f.R = -f.R;
			}
		}

		// each face in 2D, on the coordinate plane where it is largest
		for (size_t i = 0; i < nf; ++i) {
			Vec3 const& N = fFaces[i].N;
			Real ax = std::fabs(N.fX), ay = std::fabs(N.fY), az = std::fabs(N.fZ);
			int drop = ax >= ay && ax >= az ? 0 : (ay >= az ? 1 : 2);
			fDrop[i] = drop;
			fU[i].clear();
			fV[i].clear();
			for (int k : fIndices[i]) {
				Vec3 const& p = fVertices[k];
				fU[i].push_back(drop == 0 ? p.fY : p.fX);
				fV[i].push_back(drop == 2 ? p.fY : p.fZ);
			}
		}
	}

	int Polyhedron::hitFace(Ray const& ray, Real& s, bool& in2out) const
	{
		int best = -1;
		Real sbest = kInfity;
		for (size_t i = 0; i < fFaces.size(); ++i) {
			PolyhedronFace const& f = fFaces[i];
			Real a = dot(f.N, ray.fD);
			if (a == 0) {
				continue;
			}
			Real t = -(dot(f.N, ray.fO) + f.R) / a;
			if (t <= gSmin || t >= sbest) {
				continue;
			}
			Vec3 p = ray.fO + ray.fD * t;
			int drop = fDrop[i];
			Real u = drop == 0 ? p.fY : p.fX;
			Real v = drop == 2 ? p.fY : p.fZ;
			// crossing number: a ray from (u, v) along +u crosses the outline
			// an odd number of times iff the point is inside; the half-open
			// test on v counts a crossing at a shared vertex once
			std::vector<Real> const& U = fU[i];
			std::vector<Real> const& V = fV[i];
			bool inside = false;
			for (size_t k = 0, m = U.size() - 1; k < U.size(); m = k++) {
				if ((V[k] > v) != (V[m] > v)) {
					Real uc = U[k] + (v - V[k]) * (U[m] - U[k]) / (V[m] - V[k]);
					if (u < uc) {
						inside = !inside;
					}
				}
			}
			if (inside) {
				best = (int)i;
				sbest = t;
				in2out = a > 0;
			}
		}
		s = sbest;
		return best;
	}

	void Polyhedron::distance(Ray const& ray, Real& s, bool& in2out) const
	{
		if (hitFace(ray, s, in2out) < 0) {
			s = kInfity;
		}
	}

	void Polyhedron::process(Ray const& ray, ProcessHandler& handler) const
	{
		Real s;
		bool in2out;
		int i = hitFace(ray, s, in2out);
		if (i >= 0) {
			report(ray, i, s, handler);
		}
	}

	std::shared_ptr<Polyhedron> polyhedron(std::vector<Vec3> vertices,
		std::vector<std::vector<int>> faces)
	{
		auto p = std::make_shared<Polyhedron>();
		p->setMesh(std::move(vertices), std::move(faces));
		return p;
	}

}
