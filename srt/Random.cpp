#include "Random.h"
#include "MirrorReflect.h"
#include <stdint.h>
#include <assert.h>
#include <math.h>
//#define SRT_CPPSTD

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

    thread_local uint64_t gDefaultRndEngine = realRandomSeed((uint32_t)(uint64_t)&gDefaultRndEngine);

    static uint64_t xorshift64(uint64_t& a)
    {
        uint64_t x = a;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        return a = x;
    }

    void setRandomSeed(uint32_t s)
    {
        gDefaultRndEngine = realRandomSeed(s);
    }

#endif

    static double asDouble(uint64_t v)
    {
        return ((uint64_t)(v >> 11)) / (double)(uint64_t(1) << 53);
    }

    Real uniformUnitary() {
#ifdef SRT_CPPSTD
        std::uniform_real_distribution<Real> ur(0, 1);
        return ur(gDefaultRndEngine);
#else
        uint64_t x = xorshift64(gDefaultRndEngine);
        double d = asDouble(x);
        return d;
#endif

    }

    // uniform random number in rnage [a,b]
    Real uniform(Real a, Real b)
    {
#ifdef SRT_CPPSTD
        std::uniform_real_distribution<Real> ur(a, b);
        return ur(gDefaultRndEngine);
#else
        uint64_t x = xorshift64(gDefaultRndEngine);
        double d = asDouble(x);
        return d * (b - a) + a;
#endif
    }


    void randomSinCos(Real & sinphi,
        Real &cosphi) {
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

    // halfAngle = gPi/2
    Vec3 randomDiffuseRay(Vec3 const& N)
    {
        Real cdf = uniform(0, 1);
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
