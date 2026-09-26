#ifndef SRT_BOUND_H
#define SRT_BOUND_H

#include "Real.h"
#include "Vec3.h"

namespace srt {

    struct Bound {

        virtual bool onInBound(Vec3 const& p) const = 0;
        virtual ~Bound() {}
        bool inBound(Vec3 const& p) const;

        // which kind of bound this is, so that inBound() can test the
        // common one without a virtual call
        enum class Kind {
            Other,
            Box,    // BoxBound
        };
        Kind fKind = Kind::Other;
    };

    struct BoxBound : Bound
    {

        BoxBound(Real x0, Real x1, Real y0, Real y1, Real z0, Real z1)
        {
            fKind = Kind::Box;
            fX0 = x0;
            fX1 = x1;
            fY0 = y0;
            fY1 = y1;
            fZ0 = z0;
            fZ1 = z1;
        }

        Real fX0 = -kInfity;
        Real fX1 = +kInfity;
        Real fY0 = -kInfity;
        Real fY1 = +kInfity;
        Real fZ0 = -kInfity;
        Real fZ1 = +kInfity;

        void setXBound(Real x0, Real x1) { fX0 = x0; fX1 = x1; }
        void setYBound(Real x0, Real x1) { fY0 = x0; fY1 = x1; }
        void setZBound(Real x0, Real x1) { fZ0 = x0; fZ1 = x1; }

        bool onInBound(Vec3 const& p) const override
        {
            return p.fX > fX0 && p.fX < fX1&&
                p.fY > fY0 && p.fY < fY1&&
                p.fZ > fZ0 && p.fZ < fZ1;
        }
    };

    inline bool Bound::inBound(Vec3 const& p) const {
        if (fKind == Kind::Box) {
            return static_cast<BoxBound const*>(this)->BoxBound::onInBound(p);
        }
        return onInBound(p);
    }

    inline bool inBound(Bound const* b, Vec3 const& p)
    {
        return !b || b->inBound(p);
    }

}
#endif
