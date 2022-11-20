#ifndef SRT_SCALER_H
#define SRT_SCALER_H

#include "Real.h"

namespace srt {

    struct Scaler
    {
        Real xMin;
        Real xMax;
        Real yMin;
        Real yMax;

        Real w;
        Real h;

        Real pixelWidthX() const
        {
            return (xMax - xMin) / w;
        }
        Real pixelWidthY() const
        {
            return (yMax - yMin) / w;
        }

        Real pixel2WorldX(Real x) const
        {
            return x / w * (xMax - xMin) + xMin;
        }
        Real pixel2WorldY(Real y) const
        {
            return (h - y) / h * (yMax - yMin) + yMin;
        }

        Real worldToPixelX(Real x) const
        {
            return (x - xMin) / (xMax - xMin) * w;
        }
        Real worldToPixelY(Real y) const
        {
            return (yMax - y) / (yMax - yMin) * w;
        }
    };
}

#endif
