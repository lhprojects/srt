#ifndef SRT_COLOR_H
#define SRT_COLOR_H

#include <utility>
#include "Real.h"

namespace srt {

    inline double Clip(double c) {
        if (c < 0)
            return 0;
        if (c > 1)
            return 1;
        return c;
    }

    inline void Clip(double& a, double& b, double& c) {
        a = Clip(a);
        b = Clip(b);
        c = Clip(c);
    }

    inline double RGB2sRGB(double c) {
        if (c <= 0.0031308)
            return 12.92 * c;

        double a = 0.055;
        return (1 + a) * pow(c, 1 / 2.4) - a;
    }

    inline void RGB2sRGB(double& a, double& b, double& c) {
        a = RGB2sRGB(a);
        b = RGB2sRGB(b);
        c = RGB2sRGB(c);
    }


    struct Color
    {
    private:
        Real fR;
        Real fG;
        Real fB;
        Real fA;
    public:

        Color() = default;
        Color(Real r, Real g, Real b, Real al);
        Color(Real r, Real g, Real b);

        Color operator+=(Color const& c);
        Color operator-=(Color const& c);
        Color operator/=(Real s);

        Real const& R() const { return fR; }
        Real const& G() const { return fG; }
        Real const& B() const { return fB; }
        Real const& A() const { return fA; }

        Real& R() { return fR; }
        Real& G() { return fG; }
        Real& B() { return fB; }
        Real& A() { return fA; }

        void eachmul(Real s)
        {
            fR *= s; fG *= s; fB *= s; fA *= s;
        }
        
        // return r+g+b
        Real sum() const
        {
            return fG + fR + fB;
        }

        void cmul(Real s)
        {
            fR *= s; fG *= s; fB *= s;
        }

        void cadd(Real s)
        {
            fR += s; fG += s; fB += s;
        }

        void mul(Real s)
        {
            fR *= s; fG *= s; fB *= s; fA *= s;
        }
        Real cmax() const
        {
            return std::max(fR, std::max(fG, fB));
        }

        Real cmin() const
        {
            return std::min(fR, std::min(fG, fB));
        }

        Real max() const
        {
            return std::max(std::max(fR, fA), std::max(fG, fB));
        }

        void cnormalize()
        {
            fR /= cmax();
            fG /= cmax();
            fB /= cmax();
        }

		static Color red(Real alpha = 1.) { return { 1,0,0,alpha }; }
		static Color green(Real alpha = 1.) { return { 0,1,0,alpha }; }
		static Color blue(Real alpha = 1.) { return { 0,0,1,alpha }; }
		static Color white(Real alpha = 1.) { return { 1,1,1,alpha }; }
		static Color black(Real alpha = 1.) { return { 0,0,0,alpha }; }


        void precompute()
        {
            fR *= fA;
            fG *= fA;
            fB *= fA;
        }

        void precomputeOver(Color c)
        {
            *this = {
                fR + (1 - fA) * c.fR,
                fG + (1 - fA) * c.fG,
                fB + (1 - fA) * c.fB,
                fA + (1 - fA) * c.fA,
            };
        }

        friend Color eachmul(Color const& c, Real s);
        friend Color eachadd(Color const& c, const Color& s);
        friend Color over2(Color const& c1, Color const& c2);

    };


    inline Color eachmul(Color const& c, Real s)
    {
        return { c.fR * s, c.fG * s, c.fB * s, c.fA * s };
    }

    inline Color operator*(Color const& c, Real s)
    {
        return eachmul(c, s);
    }

    inline Color eachadd(Color const& c, Color const& s)
    {
        return { c.fR + s.fR, c.fG + s.fG, c.fB + s.fB, c.fA + s.fA };
    }

    inline Color operator+(Color const& c, Color const& s)
    {
        return eachadd(c, s);
    }

    inline Color over2(Color const& c1, Color const& c2)
    {
        Real alpha = c1.fA + (1 - c1.fA) * c2.fA;
        return Color(
            (c1.fA * c1.fR + (1 - c1.fA) * c2.fA * c2.fR) / alpha,
            (c1.fA * c1.fG + (1 - c1.fA) * c2.fA * c2.fG) / alpha,
            (c1.fA * c1.fB + (1 - c1.fA) * c2.fA * c2.fB) / alpha,
            alpha);
    }

    inline Color::Color(Real r, Real g, Real b, Real al)
    {
        fR = r;
        fG = g;
        fB = b;
        fA = al;
    }

    inline Color::Color(Real r, Real g, Real b) 
        : Color(r,g,b,1) {
    }

    inline Color Color::operator+=(Color const& c)
    {
        fR += c.fR;
        fG += c.fG;
        fB += c.fB;
        fA += c.fA;
        return *this;
    }

    inline Color Color::operator-=(Color const& c)
    {
        fR -= c.fR;
        fG -= c.fG;
        fB -= c.fB;
        fA -= c.fA;
        return *this;
    }

    inline Color Color::operator/=(Real s)
    {
        fR /= s;
        fG /= s;
        fB /= s;
        fA /= s;
        return *this;
    }

}
#endif

