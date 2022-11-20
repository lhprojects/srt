#ifndef SRT_RECORDERS_H
#define SRT_RECORDERS_H

#include <vector>
#include <memory>
#include <cmath>

#include "Real.h"
#include "Recorder.h"
#include "Device.h"

namespace srt {
	extern constexpr int kDebugLight = 4;
	extern constexpr int kDebugReflect = 2;
	extern constexpr int kDebugRefract = 1;
	extern constexpr int kDebugEscape = 8;
	extern constexpr int kDebugDie = 16;
	extern constexpr int kDebugScreen = 32;
	extern constexpr int kDebugGenerate = 64;
	extern constexpr int kDebugEnd = 128;



	struct Logger : Recorder
	{
		int debugConfig;

		Logger(int debugConfig = -1) :debugConfig(debugConfig)
		{
		}

		void record(Event event, // what event
			Ray const& ray,      // new generated/reflected/refrated ray, or random value
			int level,           // level, random value for end event
			TracingHandler& handler) override;
	};


	inline std::shared_ptr<Recorder> logger(int debugConfig = -1)
	{
		return std::make_shared<Logger>(debugConfig);
	}

	struct RecorderDevice : Recorder, Device
	{
	};

	struct Tracking : RecorderDevice, SurfaceProperties
	{
		struct Node {
			Event event;
			int level;
			Vec3 o{};
			int cnt = 1;
			std::vector<std::shared_ptr<Node>> subnodes;

			template<class T>
			void for_each(T const& t)
			{
				for (auto& n : subnodes) {
					t(this, n.get());
					n->for_each(t);
				}
			}
		};

		Tracking(Real r) :
			fPictrueRadius(r)
		{

		}

		void record(Event event,
			Ray const& ray,
			int level,
			TracingHandler& handler) override;

		Real distance(Vec3 o, Vec3 d, Real len,
			Ray const& ray, Vec3& n) const;

		void process(Ray const& ray, ProcessHandler& handler) const override;
	private:

		Real fPictrueRadius = 0.01;

		std::vector<Node*> nodes;
		std::unique_ptr<Node> path;
		std::vector<std::unique_ptr<Node>> paths;
	};



	inline std::shared_ptr<RecorderDevice> tracking(Real pictrueRadius)
	{
		return std::make_shared<Tracking>(pictrueRadius);
	}

}

#endif
