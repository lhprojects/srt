#define _CRT_SECURE_NO_WARNINGS
#include "Bitmap.h"
#include "stb_image_write.h"
#include "TrueType.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace srt {


	void Bitmap::write(std::string const& filename)
	{
		ImageFileFormat fmt = getImageFileFormat(tolower(filenameExtension(filename)));
		std::ofstream os(filename, std::ios_base::binary);
		if (!os.is_open()) {
			throw "file cant not create";
		}
		write(os, fmt);
		os.close();
	}

	static void file_callback(void* ctx, void* data, int size)
	{
		std::ostream* os = (std::ostream*)ctx;
		os->write((char*)data, size);

	}

	void Bitmap::write(std::ostream& os, ImageFileFormat iff)
	{
		Bitmap bitmap;
		bitmap.resize(fW, fH);
		bitmap.fC = fC;
		bitmap.regularize();
		bitmap.RGB2sRGB();

		if (iff == ImageFileFormat::PPM) {
			os << "P3\n";
			os << fW << " " << fH << "\n";
			os << 255 << "\n";

			for (int i = 0; i < fH; ++i) {
				for (int j = 0; j < fW; ++j) {
					Color color = bitmap.at(i, j);
					os << (int)round(color.R() * 255) << " "
						<< (int)round(color.G() * 255) << " "
						<< (int)round(color.B() * 255) << "\n";
				}
			}
		}
		else if (iff == ImageFileFormat::PNG) {
			std::vector<uint8_t> data;
			data.reserve(4 * fH * fW);
			for (int i = 0; i < fH; ++i) {
				for (int j = 0; j < fW; ++j) {
					Color color = bitmap.at(i, j);
					data.push_back((int)round(color.R() * 255));
					data.push_back((int)round(color.G() * 255));
					data.push_back((int)round(color.B() * 255));
					data.push_back((int)round(color.A() * 255));
				}
			}
			stbi_write_png_to_func(file_callback, &os,
				(int)fW, (int)fH, 4, data.data(), 4 * (int)fW);
		}

	}

	struct BitmapBlit : Blit
	{
		Bitmap* bitmap{};
		Color color{};

		void blit(int x0, int y0, void* data,
			int w, int h, int stride) override
		{
			unsigned char* d = (unsigned char*)data;
			for (int i = 0; i < h; ++i) {
				for (int j = 0; j < w; ++j) {
					if (x0 + j < (int)bitmap->fW && y0 + i < (int)bitmap->fH
						&& x0 + j >= 0 && y0 + i >= 0) {
						Real alpha = d[i * stride + j] / 255.;
						Color& bkgc = bitmap->at(y0 + i, x0 + j);
						Color c = color;
						c.A() *= alpha;
						bkgc = over2(c, bkgc);
					}
				}
			}
		}

	};

	void Bitmap::draw(char const* text,
		TextPaint const& paint,
		Real x0, Real y0)
	{
		BitmapBlit tb;
		tb.bitmap = this;
		tb.color = paint.fColor;

		auto get_font_folder = []() {
#ifdef _MSC_VER
			return std::string("C:\\Windows\\Fonts");
#else
			return "/usr/share/fonts";
#endif
		};
		static TrueType tt(get_font_folder() + "/times.ttf");

		int xshift = 0;
		if (paint.fHorizentalAlign == HorizentalAlign::Center) {
			int len = (int)tt.length(text, paint.fFontSize);
			xshift = -len / 2;
		}
		else if (paint.fHorizentalAlign == HorizentalAlign::Right) {
			int len = (int)tt.length(text, paint.fFontSize);
			xshift = -len;
		}

		if (paint.fVerticalAlign == VerticalAlign::Top) {
			tt.draw(text, paint.fFontSize,
				int(x0 + xshift), (int)y0, (int)fW, tb);
		}
		else if (paint.fVerticalAlign == VerticalAlign::Center) {
			tt.draw(text, paint.fFontSize,
				int(x0 + xshift), int(y0 - paint.fFontSize / 2), (int)fW, tb);
		}
		else if (paint.fVerticalAlign == VerticalAlign::Bottom) {
			tt.draw(text, paint.fFontSize,
				int(x0 + xshift), int(y0 - paint.fFontSize), (int)fW, tb);
		}
	}

	void Bitmap::setBlack(Real alpha)
	{
		for (int i = 0; i < fH; ++i) {
			for (int j = 0; j < fW; ++j) {
				at(i, j) = Color::black(alpha);
			}
		}
	}


	void Bitmap::gamma_correct(Real gamma)
	{
		for (auto& c : fC) {
			c.G() = pow(c.G(), gamma);
			c.R() = pow(c.R(), gamma);
			c.B() = pow(c.B(), gamma);
		}
	}

	void Bitmap::normalize()
	{
		Real maxc = max();
		for (auto& c : fC) {
			c.mul(1. / maxc);
		}
	}

	void Bitmap::cnormalize()
	{
		Real maxc = cmax();
		if (maxc > 0) {
			for (auto& c : fC) {
				c.cmul(1. / maxc);
			}
		} else {
			for (auto& c : fC) {
				c.black(c.A());
			}
		}
	}

	void Bitmap::cclip(Real max_)
	{

		assert(max_ >= 0);
		for (auto& c : fC) {
			Real s = c.sum();
			if (s > max_) c.cmul(max_ / s);
		}
	}

	void Bitmap::setAlpha(Real a)
	{
		for (auto& c : fC) {
			c.A() = a;
		}
	}

	Real Bitmap::max() const
	{
		auto b = std::max_element(fC.begin(), fC.end(),
			[](Color c1, Color c2) {
				return c1.max() < c2.max();
			});

		if (b != fC.end()) {
			return b->max();
		}
		else {
			return 1.;
		}
	}

	Real Bitmap::cmax() const
	{
		auto b = std::max_element(fC.begin(), fC.end(),
			[](Color c1, Color c2) {
				return c1.cmax() < c2.cmax();
			});

		if (b != fC.end()) {
			return b->cmax();
		} else {
			return 1.;
		}
	}

	Real Bitmap::cmin() const
	{
		auto b = std::min_element(fC.begin(), fC.end(),
			[](Color c1, Color c2) {
				return c1.cmin() < c2.cmin();
			});

		if (b != fC.end()) {
			return b->cmin();
		}
		else {
			return 1.;
		}
	}

	void Bitmap::over(Color bkg)
	{
		for (Color& c : fC) {
			c = over2(c, bkg);
		}
	}

	void Bitmap::cregularize()
	{
		for (auto& c : fC) {
			Clip(c.R(), c.G(), c.B());
		}
	}

	void Bitmap::regularize() {
		for (auto& c : fC) {
			Clip(c.R(), c.G(), c.B());
			c.A() = Clip(c.A());
		}
	}

	void Bitmap::eachcnormalize()
	{
		for (Color& c : fC) {
			c.cnormalize();
		}
	}

	void Bitmap::RGB2sRGB()
	{
		for (Color& c : fC) {
			Clip(c.R(), c.G(), c.B());
			srt::RGB2sRGB(c.R(), c.G(), c.B());
		}
	}

	void Bitmap::resize(int w, int h)
	{
		fW = w;
		fH = h;
		fC.clear();
		fC.resize(size_t(w) * h);
	}


	ImageFileFormat getImageFileFormat(std::string const& ext)
	{
		if (ext == "" || ext == "png") {
			return ImageFileFormat::PNG;
		}
		else if (ext == "ppm") {
			return ImageFileFormat::PPM;
		}
		else {
			throw "unknown file ext";
		}
	}

	struct PixelScaler {

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
			return y / h * (yMax - yMin) + yMin;
		}

		Real worldToPixelX(Real x) const
		{
			return (x - xMin) / (xMax - xMin) * w;
		}
		Real worldToPixelY(Real y) const
		{
			return (y - yMin) / (yMax - yMin) * w;
		}
	};

	Color Bitmap::average(int xi, Real y0, Real y1) const
	{
		int yfirst = (int)ceil(y0);
		int ylast = (int)floor(y1);

		if (ylast < yfirst) {
			return at(yfirst, xi);
		}
		else {
			Color t = Color::black(0);
			Real ya = yfirst - y0;
			Real yb = y1 - ylast;

			if (yfirst - 1 > 0 && ya > 0) {
				t += eachmul(at(yfirst - 1, xi), ya);
			}

			for (int yi = yfirst; yi < ylast; ++yi) {
				t += at(yi, xi);
			}

			if (ylast < fH && yb > 0) {
				t += eachmul(at(ylast, xi), yb);
			}
			return eachmul(t, 1 / (y1 - y0));
		}
	}

	Color Bitmap::average(Real x0, Real x1, Real y0, Real y1) const
	{

		int xfirst = (int)ceil(x0);
		int xlast = (int)floor(x1);

		if (xlast < xfirst) {
			return average(xfirst, y0, y1);
		}
		else {
			Real xa = ceil(x0) - x0;
			Real xb = x1 - floor(x1);

			Color t = Color::black(0);

			if (xfirst - 1 > 0 && xa > 0) {
				t += eachmul(average(xfirst - 1, y0, y1), xa);
			}

			for (int xi = xfirst; xi < xlast; ++xi) {
				t += average(xi, y0, y1);
			}

			if (xlast < fW && xb > 0) {
				t += eachmul(average(xlast, y0, y1), xb);
			}
			return eachmul(t, 1 / (x1 - x0));
		}

	}


	struct ColorCal {

		Bitmap const* bitmap = nullptr;
		PixelScaler s;

		Color c;

		Color average(int yi, int xi)
		{
			if (bitmap) {
				Real x0 = s.worldToPixelX(xi);
				Real x1 = s.worldToPixelX(Real(xi) + 1);
				Real y0 = s.worldToPixelY(yi);
				Real y1 = s.worldToPixelY(Real(yi) + 1);
				Color c = bitmap->average(x0, x1, y0, y1);
				return c;
			}
			else {
				return c;
			}
		}
	};

	struct Bliter {

		Bitmap* bitmap;
		BlitType fType;

		void blit(int yi, int xi, Color c, Real area)
		{
			if (fType == BlitType::Add) {
				bitmap->at(yi, xi) += c * area;
			}
			else if (fType == BlitType::Sub) {
				bitmap->at(yi, xi) -= c * area;
			}
			else if (fType == BlitType::ISub) {
				bitmap->at(yi, xi) = eachadd(c * area, eachmul(bitmap->at(yi, xi), 1 - 2 * area));
			}
			else if (fType == BlitType::AlphaMix) {
				Color& bkgc = bitmap->at(yi, xi);
				bkgc = eachadd(eachmul(over2(c, bkgc), area),
					eachmul(bkgc, 1 - area));
			}
		}
	};

	void drawRect(ColorCal& cc,
		Bitmap& bitmap,
		Real x0_, Real x1_, Real y0_, Real y1_,
		BlitType blitType)
	{

		Bliter bliter;
		bliter.bitmap = &bitmap;
		bliter.fType = blitType;

		int xfirst = (int)floor(x0_);
		int xlast = (int)ceil(x1_);
		int yfirst = (int)floor(y0_);
		int ylast = (int)ceil(y1_);

		int xfist_ = std::max(0, xfirst);
		int xlast_ = std::min((int)bitmap.fW, xlast);
		int yfist_ = std::max(0, yfirst);
		int ylast_ = std::min((int)bitmap.fH, ylast);

		for (int xi = xfist_; xi < xlast_; ++xi) {

			Real area = 1.;
			if (xi < x0_) {
				area *= x0_ - xi;
			}
			if (xi > x1_) {
				area *= xi - x1_;
			}

			for (int yi = yfist_; yi < ylast_; ++yi) {
				if (yi < y0_) {
					area *= y0_ - yi;
				}
				if (yi > y1_) {
					area *= yi - y1_;
				}
				Color c = cc.average(yi, xi);
				bliter.blit(yi, xi, c, area);

			}
		}
	}

	void Bitmap::draw(Color c,
		Real x0_, Real x1_, Real y0_, Real y1_,
		BlitType type)
	{
		ColorCal cc;
		cc.c = c;
		drawRect(cc, *this, x0_, x1_, y0_, y1_, type);
	}

	void Bitmap::draw(Bitmap const& bitmap,
		Real x0_, Real x1_, Real y0_, Real y1_)
	{
		ColorCal cc;
		cc.bitmap = &bitmap;
		cc.s.w = bitmap.fW;
		cc.s.h = bitmap.fH;
		cc.s.xMin = x0_;
		cc.s.xMax = x1_;
		cc.s.yMin = y0_;
		cc.s.yMax = y1_;
		drawRect(cc, *this, x0_, x1_, y0_, y1_, BlitType::AlphaMix);
	}









}
