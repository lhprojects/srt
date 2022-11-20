#include "Screen.h"
#include "Quadric.h"
#include "Surfaces.h"

namespace srt {


	enum class fileformat {
		CSV,
	};


	fileformat get_format(std::string const& file)
	{
		std::string substr = filenameExtension(file);
		tolower(substr);
		if (substr == "csv" || substr == "") {
			return fileformat::CSV;
		}
		else {
			throw "unkown file format";
		}
	}

	void Screen::save(std::string const& file)
	{

		fileformat ff = get_format(file);
		if (ff == fileformat::CSV) {
			std::ofstream of;
			of.open(file);
			of << "x,y,z,direction_x,direction_y,direction_z,amp,freq,\n";
			for (size_t i = 0; i < fRays.size(); ++i) {
				Ray const& r = fRays[i];
				of << r.fO.fX << "," << r.fO.fY << "," << r.fO.fZ << ",";
				of << r.fD.fX << "," << r.fD.fY << "," << r.fD.fZ << ",";
				of << r.fAmp << ",";
				of << r.fLambda << "\n";
			}
			of.close();
		}
		else {
			throw "unknown file format";
		}
	}


	void Screen::record(
		Ray const& r, ProcessHandler& handler) const
	{

		if (handler.fType == HandlerType::Tracing) {
			TracingHandler& th = static_cast<TracingHandler&>(handler);

			if (th.record)
			{
				if (fRecordOut2In) {
					if (dot(r.fD, th.N) < 0) { // out 2 in

						Vec3 inter = static_cast<TracingHandler&>(handler).inter;
						fRays.push_back(Ray(inter, r.fD, r.fAmp, r));
					}
				}
				if (fRecordIn2Out) {
					if (dot(r.fD, th.N) > 0) { // in 2 out

						Vec3 inter = static_cast<TracingHandler&>(handler).inter;
						fRays.push_back(Ray(inter, r.fD, r.fAmp, r));
					}
				}
			}

		}
	}

	void Raster::raster(Bitmap& bitmap,
		Iter<Ray>& iter,
		ScreenOpts const& opts)
	{
		Scaler s;
		s.h = opts.High;
		s.w = opts.Width;
		s.xMin = opts.N1Min;
		s.xMax = opts.N1Max;
		s.yMin = opts.N2Min;
		s.yMax = opts.N2Max;

		bitmap.resize(opts.Width, opts.High);
		bitmap.setBlack(0);

		for (; !iter.end();) {
			Ray p = iter.get();
			Real x = s.worldToPixelX(dot(p.fO - opts.Origin, opts.N1));
			Real y = s.worldToPixelY(dot(p.fO - opts.Origin, opts.N2));
			int iidx = (int)floor(x);
			int jidx = (int)floor(y);
			if (iidx < opts.Width && jidx < opts.High
				&& iidx >= 0 && jidx >= 0) {

				Color c;
				if (opts.Gray) {
					c = Color::white(1.);
				}
				else {
					WaveLength2RGB(p.fLambda, &c.R(), &c.G(), &c.B());
				}
				c.cmul(p.fAmp);
				bitmap.at(jidx, iidx) += c;

			}
			iter.next();
		}
		bitmap.cnormalize();
		bitmap.setAlpha(1.);
	}

	void Screen::raster(Bitmap& bitmap, ScreenOpts const& opts)
	{
		auto ran = range(fRays);
		Raster rast;
		rast.raster(bitmap, ran, opts);
	}

	void Screen::raster(std::ostream& os,
		ImageFileFormat iff,
		ScreenOpts const& opts)
	{
		Bitmap bmp;
		raster(bmp, opts);
		bmp.write(os, iff);
	}

	void Screen::raster(std::string const& filename,
		ScreenOpts const& opts)
	{
		std::ofstream os(filename, std::ios_base::binary);
		ImageFileFormat fmt = getImageFileFormat(tolower(filenameExtension(filename)));
		raster(os, fmt, opts);
		os.close();
	}

	QuadricScreen::QuadricScreen() : QuadricSurface()
	{
	}

	void QuadricScreen::process(Ray const& in, ProcessHandler& handler) const
	{
		QuadricSurface::process(in, handler);
		record(in, handler);
	}


	void PlaneScreen::process(Ray const& in, ProcessHandler& handler) const
	{
		PlaneSurface::process(in, handler);
		record(in, handler);
	}

}
