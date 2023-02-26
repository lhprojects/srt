#include "Vec3.h"
#include <format>

namespace srt {
	std::string to_string(Vec3 const& q) {
		return std::format("[{:9} {:9} {:9}]",
			q.fX, q.fY, q.fZ);
	}
}
