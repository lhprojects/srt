#pragma once
#include "../Vec3.h"

namespace srt {
    struct Stop {
        virtual Vec3 sample() = 0;
        virtual Vec3 sample(Vec3 const& ref) = 0;
        virtual bool accept(Vec3 const& ref,
            Vec3 const& d) = 0;
        virtual Real solidAngle(Vec3 const& ref) = 0;
    };
}
