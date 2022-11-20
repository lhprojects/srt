#pragma once

namespace srt {

	enum class ReflectType {
		// reflect, refract ratio defined by your setting
		Diffuse,
		Metal,
		Mirror,
		// reflect, refract ratio defined by index
		// reflect, refract ratio you set will not be used
		Optical,
		Rayleigh,
	};
}
