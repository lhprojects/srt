#ifndef SRT_REAL_H
#define SRT_REAL_H

#include <limits>
#include <cstdint>

namespace srt {


	using Int = std::int64_t;
	using Real = double;
	constexpr Real kPi = 3.1415926535898;
	constexpr Real kInfity = std::numeric_limits<Real>::infinity();

	template<class S>
	S Sqr(S x) { return x * x; }


}
#endif
