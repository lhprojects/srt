#ifndef SRT_PAINT_H
#define SRT_PAINT_H

#include "Align.h"
#include "Real.h"
#include "Color.h"
#include "Pars.h"

namespace srt {

    struct Paint
    {
		Color fColor = Color::black();
		using Pars = pars::Pars<pars::color_>;

        Paint(pars::uncheck_t, pars::argument auto const &... args)
        {
            set(pars::uncheck, args...);
        }

        void set(pars::uncheck_t, pars::argument auto const &... args)
        {
            pars::set(fColor, pars::color, args...);
        }

    };


    struct TextPaint : Paint
    {
        // 
        // pars::fontSize
        double fFontSize = 30;
        VerticalAlign fVerticalAlign = VerticalAlign::Top;
        HorizentalAlign fHorizentalAlign = HorizentalAlign::Left;

		using Pars = pars::MergePars<Paint::Pars,
			pars::Pars<pars::fontSize_, pars::verticalAlign_, pars::horizentalAlign_>
		>;

        TextPaint(pars::argument auto const &... args)
            : Paint(pars::uncheck, args...)
        {
            pars::check<Pars>(args...);
            set(pars::uncheck, args...);
        }

        TextPaint(pars::uncheck_t, pars::argument auto const &... args)
            : Paint(pars::uncheck, args...) {
            set(pars::uncheck, args...);
        }

        void set(pars::argument auto const &... args)
        {
            pars::check<Pars>(args...);
            set(pars::uncheck, args...);
        }

        void set(pars::uncheck_t, pars::argument auto const &... args)
        {
            pars::set(fFontSize, pars::fontSize, args...);
            pars::set(fVerticalAlign, pars::verticalAlign, args...);
            pars::set(fHorizentalAlign, pars::horizentalAlign, args...);
            Paint::set(pars::uncheck, args...);
        }

    };


}
#endif
