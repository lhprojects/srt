#ifndef SRT_RANDOM_H
#define SRT_RANDOM_H

#include "Real.h"
#include "Vec3.h"
#include <stdint.h>
#include <math.h>
#include <cmath>
//#define SRT_CPPSTD

namespace srt {

#ifdef SRT_CPPSTD
    Real uniformUnitary();
    // uniform random number in rnage [a,b]
    Real uniform(Real a, Real b);
#else
    // xorshift64 state of this thread; 0 until its first number is drawn
    inline thread_local uint64_t gRandomState = 0;
    // seeds gRandomState and returns it
    uint64_t randomSeedThisThread();

    inline uint64_t randomNext()
    {
        uint64_t x = gRandomState;
        if (x == 0) {
            x = randomSeedThisThread();
        }
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        return gRandomState = x;
    }

    inline Real uniformUnitary()
    {
        return (randomNext() >> 11) / (double)(uint64_t(1) << 53);
    }

    // uniform random number in rnage [a,b]
    inline Real uniform(Real a, Real b)
    {
        return uniformUnitary() * (b - a) + a;
    }
#endif

    inline void randomSinCos(Real& sinphi,
        Real& cosphi)
    {
#if 0
        Real phi = uniform(0, 2 * kPi);
        sinphi = sin(phi);
        cosphi = cos(phi);
#else
        Real sin2phi;
        Real cos2phi;

        for (;;) {
            Real x = uniform(-1, 1);
            Real y = uniform(-1, 1);
            Real r2 = x * x + y * y;
            if (r2 <= 1) {
                Real r2i = 1 / r2;
                sin2phi = 2. * x * y * r2i;
                cos2phi = (x * x - y * y) * r2i;
                break;
            }
        }
        sinphi = sin2phi;
        cosphi = cos2phi;
#endif
    }

    // return a random vector normal to d
    // uniformly in theta
    Vec3 randomNorm(Vec3 const& d);

    // return a uniformly random ray (in solid angle).
    // inside the coin which axis is N and which half angle is halfAngle.
    //        /       half angle -- halfAngle
    // coin  . ----   axis -- N
    //        \       half angle -- halfAngle
    //
    Vec3 randomUniformRay(Vec3 const& N, Real halfAngle);

    // return a random ray diffused by the diffusion surface.
    // inside the coin which axis is N and which half angle is halfAngle (see randomUniformRay).
    Vec3 randomDiffuseRay(Vec3 const& N, Real halfAngle);
    // the same as `Vec3 randomDiffuseRay(Vec3 const& N, Real halfAngle);` but halfAngle = gPi/2
    inline Vec3 randomDiffuseRay(Vec3 const& N)
    {
        // Malley's method: a point uniform on the unit disk, lifted onto
        // the hemisphere, is cosine distributed
        Real x, y, r2;
        do {
            x = uniform(-1, 1);
            y = uniform(-1, 1);
            r2 = x * x + y * y;
        } while (r2 > 1);
        // orthonormal basis around N without branches or sqrt
        // (Duff et al., Building an Orthonormal Basis, Revisited, 2017)
        Real sign = std::copysign(Real(1), N.fZ);
        Real a = -1 / (sign + N.fZ);
        Real b = N.fX * N.fY * a;
        Vec3 n1 = { 1 + sign * N.fX * N.fX * a, sign * b, -sign * N.fX };
        Vec3 n2 = { b, sign + N.fY * N.fY * a, -N.fY };
        return x * n1 + y * n2 + sqrt(1 - r2) * N;
    }

    // return a random ray diffused by the metal surface.
    // n is the parameter for metal surface.
    Vec3 randomMetalRay(Vec3 const& rayD,
        Vec3 const& N, Real n);

}

#endif
