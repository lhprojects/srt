#ifndef SRT_RECORDER_H
#define SRT_RECORDER_H

#include "Real.h"
#include "Ray.h"
#include "Device.h"
#include <memory>
#include <vector>
#include <cmath>

namespace srt {

	enum class Event
	{
		Generate, // ray creacted
		Refract,  // ray refracted
		Reflect,  // ray reflected
		Escape,   // ray escape from the world
		Die,      // ray died
		End,      // end of ray life cycle
	};

	struct Recorder
	{
		virtual void record(Event event,
			Ray const& ray,
			int level,
			TracingHandler& handler) = 0;
	};
}

#endif

