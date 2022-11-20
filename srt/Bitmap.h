#pragma once
#include <vector>
#include <algorithm>
#include <utility>
#include <assert.h>
#include "Real.h"
#include "Vec3.h"
#include "Color.h"
#include "Align.h"
#include "wavelength.h"
#include "Paint.h"

namespace srt {

	inline void tolower_inplace(std::string& s) {
		for (size_t i = 0; i < s.size(); ++i) {
			s[i] = std::tolower(s[i]);
		}
	}

	inline std::string tolower(std::string&& s) {
		tolower_inplace(s);
		return std::move(s);
	}

	inline std::string tolower(std::string const& s) {
		std::string ns = s;
		tolower_inplace(ns);
		return std::move(ns);
	}

	inline std::string filenameExtension(std::string const& file) {
		size_t dotp = file.rfind(".");
		if (dotp == std::string::npos) {
			return "";
		} else {
			return file.substr(dotp + 1);
		}
	}

	enum class ImageFileFormat {
		PNG,
		PPM,
	};

	ImageFileFormat getImageFileFormat(std::string const& ext);


	enum class BlitType {
		AlphaMix,
		Add,
		Sub,
		ISub,
	};

	struct Bitmap {

		int fW = 0;
		int fH = 0;
		std::vector<Color> fC;


		void gamma_correct(Real gamma);
		void setAlpha(Real a);

		// clip color
		void cclip(Real max_);
		// normalize color(r,g,b)
		void cnormalize();


		void normalize();
		Real max() const;
		Real cmax() const;
		Real cmin() const;
		void over(Color c);

		void regularize();
		void cregularize();

		void eachcnormalize();
		void RGB2sRGB();

		void resize(int w, int h);
		Color& at(int i, int j);
		Color const& at(int i, int j) const;

		void write(std::ostream& os, ImageFileFormat iff);
		void write(std::string const& filename);

		Color average(int xi, Real y0, Real y1) const;
		Color average(Real x0, Real x1, Real y0, Real y1) const;

		void draw(char const* text,
			TextPaint const& paint,
			Real x0, Real y0);

		void draw(Color c,
			Real x0, Real x1, Real y0, Real y1,
			BlitType type = BlitType::AlphaMix);

		void draw(Bitmap const& bitmap,
			Real x0, Real x1, Real y0, Real y1);

		void setBlack(Real alpha = 1.);
	};

	inline Color& Bitmap::at(int i, int j) {
		assert(i < fH);
		assert(j < fW);
		return fC.at(size_t(i) * fW + j);
	}

	inline Color const& Bitmap::at(int i, int j) const {
		return fC[size_t(i) * fW + j];
	}


}

