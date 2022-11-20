
#include "Recorders.h"

namespace srt {



	template<int FLAGS>
	void debug_log(int debugconfig, char const* info,
		Ray const& ray,
		int level,
		Vec3 const& inter,
		TracingHandler& handler)
	{
		if (debugconfig & FLAGS) {
			char const* title = "";
			if (FLAGS & kDebugReflect) {
				title = "reflect";
			}
			else if (FLAGS & kDebugRefract) {
				title = "refract";
			}
			else if (FLAGS & kDebugLight) {
				title = "light";
			}
			else if (FLAGS & kDebugDie) {
				title = "die";
			}
			else if (FLAGS & kDebugEscape) {
				title = "escape";
			}
			else if (FLAGS & kDebugGenerate) {
				title = "generate";
				printf("[%3d/%2d] %8s %8s r(% 8f % 8f % 8f) v(%f %f %f) %f\n",
					(int)ray.fID,
					level,
					title, info,
					inter.fX,
					inter.fY,
					inter.fZ,
					ray.fD.fX,
					ray.fD.fY,
					ray.fD.fZ,
					ray.fLambda
				);

			}
			else if (FLAGS & kDebugEnd) {
				title = "end";
			}

			if (FLAGS & kDebugRefract) {
				char const* side = "outer";
				if (handler.inner) side = "inner";
				Real indexInner = handler.property->fIndexInner.get(ray.fLambda);
				Real indexOuter = handler.property->fIndexOuter.get(ray.fLambda);
				printf("[%3d/%2d] %8s %8s r(% 8f % 8f % 8f) v(%f %f %f) %7s [I%f/O%f]\n",
					(int)ray.fID,
					level,
					title,
					info,
					inter.fX,
					inter.fY,
					inter.fZ,
					ray.fD.fX,
					ray.fD.fY,
					ray.fD.fZ,
					side,
					indexInner,
					indexOuter
				);
			}
			if (FLAGS & kDebugReflect) {
				char const* side = "outer";
				if (handler.inner) side = "inner";
				printf("[%3d/%2d] %8s %8s r(% 8f % 8f % 8f) v(%f %f %f) %7s\n",
					(int)ray.fID,
					level,
					title,
					info,
					inter.fX,
					inter.fY,
					inter.fZ,
					ray.fD.fX,
					ray.fD.fY,
					ray.fD.fZ,
					side
				);
			}
			if (FLAGS & kDebugEscape) {
				printf("[%3d/%2d] %8s %8s r(% 8f % 8f % 8f) v(%f %f %f)\n",
					(int)ray.fID,
					level,
					title, info,
					inter.fX,
					inter.fY,
					inter.fZ,
					ray.fD.fX,
					ray.fD.fY,
					ray.fD.fZ
				);
			}
			else if(FLAGS & kDebugLight)  {
				printf("[%3d/%2d] %8s %8s r(% 8f % 8f % 8f) v(%f %f %f)\n",
					(int)ray.fID,
					level,
					title, info,
					inter.fX,
					inter.fY,
					inter.fZ,
					ray.fD.fX,
					ray.fD.fY,
					ray.fD.fZ
				);
			}
			else if (FLAGS & kDebugEnd) {
				printf("[%3d/  ] %8s %8s\n",
					(int)ray.fID,
					title,
					info
				);
			}
			else if (FLAGS & kDebugDie) {
				printf("[%3d/%2d] %8s %8s r(% 8f % 8f % 8f)\n",
					(int)ray.fID,
					level,
					title, info,
					inter.fX,
					inter.fY,
					inter.fZ
				);
			}
		}
	}

	template<int FLAGS>
	void debug_log(
		int debugConfig,
		Ray const& ray,
		int level,
		TracingHandler& handler)
	{

		debug_log<FLAGS>(debugConfig,
			(handler.device ? handler.device->getName().c_str() : ""),
			ray, level, handler.inter, handler);
	}



	void Logger::record(Event event,
		Ray const& ray,
		int level,
		TracingHandler& handler)
	{
		if (event == Event::Generate) {
			debug_log<kDebugGenerate>(debugConfig,
				(handler.device ? handler.device->getName().c_str() : ""),
				ray, level, ray.fO, handler);
		}
		else if (event == Event::Reflect) {
			debug_log<kDebugReflect>(debugConfig, ray, level, handler);
		}
		else if (event == Event::Refract) {
			debug_log<kDebugRefract>(debugConfig, ray, level, handler);
		}
		else if (event == Event::Escape) {
			debug_log<kDebugEscape>(debugConfig, ray, level, handler);
		}
		else if (event == Event::Die) {
			debug_log<kDebugDie>(debugConfig, ray, level, handler);
		}
		else if (event == Event::End) {
			debug_log<kDebugEnd>(debugConfig, ray, level, handler);
		}
	}



	void Tracking::record(Event event,
		Ray const& ray,
		int level,
		TracingHandler& handler)
	{
		if (!handler.pictrue) {
			if (event == Event::End) {
				paths.push_back(std::move(path));
				nodes.clear();
			} if (event == Event::Generate) {
				path.reset(new Node{ event, level, ray.fO });
				nodes.push_back(path.get());
			}
			else if (event == Event::Escape) {
				auto last = nodes.back();

				last->subnodes.emplace_back(new Node{ event, level, ray.fD });

				nodes.back()->cnt -= 1;
				if (nodes.back()->cnt == 0)
					nodes.pop_back();
			}
			else if (event == Event::Die) {
				auto last = nodes.back();

				last->subnodes.emplace_back(new Node{ event, level, handler.inter });

				nodes.back()->cnt -= 1;
				if (nodes.back()->cnt == 0)
					nodes.pop_back();
			}
			else if (event == Event::Reflect || event == Event::Refract) {

				if (nodes.back()->level + 1 == level) {
					auto last = nodes.back();
					last->subnodes.emplace_back(new Node{ event, level, handler.inter });

					nodes.back()->cnt -= 1;
					if (nodes.back()->cnt == 0)
						nodes.pop_back();

					nodes.emplace_back(last->subnodes.back().get());
				}
				else {
					nodes.back()->cnt += 1;
				}

			}
		}
	}

	Real Tracking::distance(Vec3 o, Vec3 d, Real len,
		Ray const& ray, Vec3& n) const
	{
		Real d3_ = dot(ray.fD, d);
		Vec3 d12_ = ray.fD - d * d3_;
		Real d12_n2 = norm2(d12_);

		Vec3 d12 = normalize(d12_);

		o = o - ray.fO;
		Real o3_ = dot(o, d);
		Vec3 o12 = o - d * o3_;

		Real l = dot(o12, d12);
		if (l > 0) {
			Real minD2 = norm2(o12) - Sqr(l);
			if (minD2 <= Sqr(fPictrueRadius)) {

				Real s = l / sqrt(d12_n2);
				Real l2 = dot(s * ray.fD - o, d);
				if (l2 > 0 && l2 <= len) {
					// trinagle <delta, fPictrueRadius, minD>
					Real delta = sqrt(Sqr(fPictrueRadius) - minD2);
					n = normalize(d12 * (l - delta) - o12);
					return s;
				}
			}
		}
		return kInfity;
	}

	void Tracking::process(Ray const& ray, ProcessHandler& handler) const
	{

		{
			Vec3 N = Vec3{ 0,0, 1 };
			Real smin = kInfity;
			if (0) {
				paths[0]->for_each([=](Node* o, Node* node) {
					printf("%d %d (%f %f %f)\n",
						o->level, node->level,
						node->o.fX, node->o.fY, node->o.fZ);
					});
			}

			for (size_t i = 0; i < paths.size(); ++i) {
				auto& path = paths[i];
				path->for_each([&](Node* o, Node* node) {
					Real s;
					if (node->event == Event::Escape) {
						s = distance(o->o, normalize(node->o),
							kInfity, ray, N);
					}
					else {
						s = distance(o->o, normalize(node->o - o->o),
							sqrt(norm2(node->o - o->o)), ray, N);
					}
					if (s < smin) {
						//printf("%f %f %f", ray.fD.fX, ray.fD.fY, ray.fD.fZ);
						smin = s;
					}
					});
			}

			if (!std::isinf(smin)) {
				if (handler.fType == HandlerType::Distance) {
					static_cast<DistanceHandler&>(handler).distance(smin, false);
				}
				else {
					Vec3 inter = ray.fO + ray.fD * smin;
					bool inner = false;
					static_cast<TracingHandler&>(handler).hitSurface(inter, N, inner,
						this, this);
				}
			}

		}
	}
}
