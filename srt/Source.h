#pragma once

#include "Real.h"
#include "Ray.h"

namespace srt {

    struct Source
    {

        Source(Real amp);

        // generate a ray
        virtual Ray generate() = 0;

        Real fAmp = 1.;
    };

}

// implementations
namespace srt {

    inline Source::Source(Real amp) : fAmp(amp)
    {
    }
}

