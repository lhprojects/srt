#include "../Pars.h"

namespace srt {
	namespace pars {
		struct directionSampler_; constexpr par<directionSampler_, std::shared_ptr<DirectionSampler>> directionSampler{};
		struct positionSampler_; constexpr par<positionSampler_, std::shared_ptr<PositionSampler>> positionSampler{};
	}
}