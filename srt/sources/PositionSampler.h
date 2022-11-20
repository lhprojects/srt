#pragma once
#include <memory>
#include "../Vec3.h"

namespace srt {

    struct PositionSampler {
        // d Prob = pdf * dA
        // return the position
        // norm set to the normal line of the surface
        // norm must be normalized
        virtual Vec3 sample(Vec3& norm) = 0;
    };

    // point position sampler, still have the norm of surface
    std::shared_ptr<PositionSampler> pointPositionSampler(Vec3 const& o,
        Vec3 const& d);
}
