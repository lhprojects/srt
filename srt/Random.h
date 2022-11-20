#ifndef SRT_RANDOM_H
#define SRT_RANDOM_H

#include "Real.h"
#include "Vec3.h"

namespace srt {
    
    Real uniformUnitary();
    // uniform random number in rnage [a,b]
    Real uniform(Real a, Real b);

    void randomSinCos(Real& sinphi,
        Real& cosphi);

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
    Vec3 randomDiffuseRay(Vec3 const& N);

    // return a random ray diffused by the metal surface.
    // n is the parameter for metal surface.
    Vec3 randomMetalRay(Vec3 const& rayD,
        Vec3 const& N, Real n);

}

#endif
