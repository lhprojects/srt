#ifndef SRT_BOUND_H
#define SRT_BOUND_H

#include "Real.h"
#include "Vec3.h"

namespace srt {

    struct Bound {

        virtual bool onInBound(Vec3 const& p) const = 0;
        virtual ~Bound() {}
        bool inBound(Vec3 const& p) const;
    };

    inline bool Bound::inBound(Vec3 const& p) const {
        return onInBound(p);
    }

    inline bool inBound(Bound const* b, Vec3 const& p)
    {
        return !b || b->inBound(p);
    }

}
#endif
