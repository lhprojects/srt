#include "Random.h"
#include "MirrorReflect.h"
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <cmath>

#ifdef SRT_CPPSTD
#include <random>
#endif

namespace srt {

#ifdef SRT_CPPSTD
    using RandomEngine = std::default_random_engine;
    thread_local RandomEngine gDefaultRndEngine;
    void setRandomSeed(uint32_t s)
    {
        gDefaultRndEngine.seed(s);
    }

#else
    static uint64_t realRandomSeed(uint32_t s)
    {
        return s & 0xa4b097a27e0f4d;
    }

    uint64_t randomSeedThisThread()
    {
        return gRandomState = realRandomSeed((uint32_t)(uint64_t)&gRandomState);
    }

    void setRandomSeed(uint32_t s)
    {
        gRandomState = realRandomSeed(s);
    }

#endif

#ifdef SRT_CPPSTD
    Real uniformUnitary() {
        std::uniform_real_distribution<Real> ur(0, 1);
        return ur(gDefaultRndEngine);
    }

    // uniform random number in rnage [a,b]
    Real uniform(Real a, Real b)
    {
        std::uniform_real_distribution<Real> ur(a, b);
        return ur(gDefaultRndEngine);
    }
#endif

    Vec3 randomNorm(Vec3 const& d)
    {
        assert(fabs(norm2(d) - 1) < 1E-10);
#if 1
        Real sinphi;
        Real cosphi;
        randomSinCos(sinphi, cosphi);

        Vec3 n1 = getNorm(d);
        Vec3 n2 = cross(n1, d);
        return sinphi * n1 + cosphi * n2;
#else
        for (;;) {
            Vec3 r = { uniform(-1, 1), uniform(-1, 1), uniform(-1, 1) };
            if (norm2(r) < 1.) { // inside the sphere
                r = r - dot(r, d) * d;
                r = normalize(r);
                return r;
            }
        }
#endif
    }

    Vec3 randomUniformRay(Vec3 const& fD, Real halfAngle)
    {
        Real r2 = Sqr(sin(0.5 * halfAngle));
        Real tsshs = uniform(0, 1) * r2;
        Real cs = 1 - 2 * tsshs;
        Real ss = 2 * sqrt(tsshs * (1 - tsshs));

        Vec3 d = normalize(ss * randomNorm(fD) + cs * fD);
        return d;

    }


    Vec3 randomDiffuseRay(Vec3 const& N, Real halfAngle)
    {
        Real r2 = Sqr(sin(halfAngle));
        Real cdf = uniform(0, 1) * r2;
        Real cs = sqrt(1 - cdf);
        Real ss = sqrt(cdf);

        Vec3 d = ss * randomNorm(N) + cs * N;
        return d;
    }

    Vec3 randomMetalRay(Vec3 const& rayD,
        Vec3 const& N, Real n)
    {
        Vec3 out = reflectDirection(N, rayD);
        Vec3 n1 = normalize(cross(out, N));
        Vec3 n2 = normalize(cross(n1, N));


        Real a = n * n;
        Real max = 1 / (-1. - 1 + a);
        Real min = 1 / (-1. + 1 + a);

        Real const m = 20;
        for (;;) {
            Real t = uniform(0, 1) * (max - min) + min;
            Real costheta = -1 / t - a + 1;
            Real sintheta = sqrt(1 - costheta * costheta);
            Real phi = uniform(0, 2 * kPi);

            Vec3 d = out * costheta + n1 * sintheta * sin(phi)
                + n2 * sintheta * cos(phi);

            if (dot(d, N) > 0) {
                return d;
            }
        }
    }
}
