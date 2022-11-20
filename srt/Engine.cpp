#include <string>
#include <memory>
#include <functional>
#include <random>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <thread>

#include "Pars.h"
#include "Device.h"
#include "Random.h"
#include "Engine.h"
#include "Recorder.h"
#include "Source.h"
#include "Scaler.h"
#include "MirrorReflect.h"

namespace srt {

	Real gSmin = 1E-8;

	void PictureOpts::lookAt(Vec3 o) {
		Vec3 r = normalize(o - Origin);
		Vec3 z = { 0,0,1 };
		Vec3 n2 = z - dot(z, r) * r;
		if (norm2(n2) == 0) {
			if (r.fZ < 0)
				n2 = { 0,1,0 };
			else
				n2 = { 0,-1,0 };
		} else {
			n2 = normalize(n2);
		}
		Vec3 n1 = cross(r, n2);
		N1 = n1;
		N2 = n2;
		FocalDistance = sqrt(norm2(o - Origin));
	}

	struct state {
		Real index;
		bool inObject;
		Ray ray;
	};

	static Device* minSDevice(std::vector<Device*>& devs,
		Ray const& ray,
		Real& ref_smin) {
		Real smin = std::numeric_limits<Real>::infinity();
		Device* smin_dev = nullptr;
		bool smin_in2out = false;

		DistanceHandler handler;

		for (size_t dev_idx = 0; dev_idx < devs.size(); ++dev_idx) {
			Device* dev = devs[dev_idx];

			handler.fDistance = kInfity;
			dev->process(ray, handler);
			Real s = handler.fDistance;

			if (!std::isinf(s)) {
				if (s < smin - gSmin) {
					smin = s;
					smin_dev = dev;
					smin_in2out = handler.fIn2out;
				} else if (s < smin + gSmin
					&& !handler.fIn2out
					&& smin_in2out) {
					smin = s;
					smin_dev = dev;
					smin_in2out = handler.fIn2out;
				} else if (s < smin) {
					smin = s;
					smin_dev = dev;
					smin_in2out = handler.fIn2out;
				}
			}
		}
		ref_smin = smin;
		return smin_dev;
	}

	static bool emitRay(std::vector<Device*>& devs,
		Ray const& ray,
		ProcessHandler& handler) {
		Real smin = kInfity;
		Device* smin_dev = minSDevice(devs, ray, smin);
		if (smin_dev) {
			smin_dev->process(ray, handler);
			return true;
		} else {
			return false;
		}
	}

	static bool checkRefractDirection(Vec3 N, Vec3 const& rayD,
		Real fromIndex, Real toIndex) {
		if (fromIndex > toIndex) {
			Vec3 N1 = cross(N, rayD);
			if (norm2(N1) * Sqr(fromIndex) >= Sqr(toIndex)) {
				return false;
			}
		}
		return true;
	}

	static bool transDirection(Vec3 N, Vec3 const& rayD,
		Real fromIndex, Real toIndex, Vec3& d) {
		// out = a*N + b*r.fD
		// b toIndex (N x out) = fromIndex (N x out)
		// b = fromIndex/toIndex
		// a^2 + 2bk a + b^2 - 1= 0
		// a = -bk +- sqrt((bk)^2 - (b^2-1))
		// a = -bk +- sqrt(1 - b^2(1-k^2))
		// a = -b cFrom + sqrt(1 - b^2 sFrom^2)

		if (fromIndex != toIndex) {
			Real b = fromIndex / toIndex;
			if (dot(N, rayD) < 0) {
				N = -N;
			}

			Vec3 N1 = cross(N, rayD);
			Real sFrom = sqrt(norm2(N1));
			Real sOuter = sFrom * b;
			Real delta = 1 - sOuter * sOuter;
			if (delta <= 0) {
				// total internal reflection   
				return false;
			} else {
				Real cFrom = dot(N, rayD);
				Real a = -b * cFrom + sqrt(delta);
				d = a * N + b * rayD;
				return true;
			}
		} else {
			d = rayD;
			return true;
		}
	}


	struct BinarySelecter {

		std::uniform_real_distribution<Real> ud;

		bool operator()(Real a, Real b) {
			Real r = uniform(0, 1);
			return r < a / (a + b);
		}
	};

	struct TraceOpts {
		int max_level = 100;
		Real min_ray_amp = 1E-6;
		int first_level_split = 1;
	};

	struct Frame {
		Ray ray;
		int level;
	};

	struct RayTracing {
		std::vector<Frame> frames;

		TraceOpts opts;
		TracingHandler handler;
		Real pixelAmp;
		std::vector<Device*>& devs;

		void init_recorder(std::vector<Recorder*> const& recorders) {
			if (recorders.size()) {
				recorder = [&](Event e, Ray const& ray, int level, TracingHandler& th) {
					for (auto r : recorders) {
						r->record(e, ray, level, th);
					}
				};
			}
		}
		std::function<void(Event e, Ray const& ray, int level, TracingHandler& th)> recorder;

		RayTracing(std::vector<Device*>& devs,
			std::vector<Recorder*>& recorders) :devs(devs) {
			init_recorder(recorders);
		}

		void traceRay(Ray const& ray);
	};

	struct ProcessReflection {
		RayTracing& rt;

		Real fromIndex = 1.;
		Real toIndex = 1.;
		Vec3 N{};
		Vec3 const& inter;
		Ray const& ray;
		int the_level;
		bool die = true;

		ProcessReflection(RayTracing& rt,
			Ray const& ray,
			int the_level) :
			rt(rt),
			inter(rt.handler.inter),
			ray(ray), the_level(the_level) {
		}

		void newRay(Ray const& nr, Event event) {
			rt.frames.emplace_back(nr, the_level + 1);
			if (rt.recorder) {
				rt.recorder(event, nr, the_level + 1, rt.handler);
			}
			die = false;
		}

		void doMirrorReflect(Real reflect,
			Vec3 const& p) {
			Vec3 rd = reflectDirection(N, ray.fD);
			Ray newRay(inter, rd, reflect * ray.fAmp, ray);
			newRay.fP = p;
			this->newRay(newRay, Event::Reflect);
		}

		void doMirrorTrans(Real trans,
			Vec3 const& p) {
			Vec3 td;
			if (transDirection(N, ray.fD,
				fromIndex, toIndex, td)) {
				Ray newRay(inter, td, trans * ray.fAmp, ray);
				newRay.fP = p;
				this->newRay(newRay, Event::Refract);
			}
		}

		void doMirrorReflect(Real reflect) {
			Vec3 rd = reflectDirection(N, ray.fD);
			Ray newRay(inter, rd, reflect * ray.fAmp, ray);
			newRay.fP = randomNorm(rd);
			this->newRay(newRay, Event::Reflect);
		}

		void doMirrorTrans(Real trans) {
			Vec3 td;
			if (transDirection(N, ray.fD,
				fromIndex, toIndex, td)) {
				Ray newRay(inter, td, trans * ray.fAmp, ray);
				newRay.fP = randomNorm(td);
				this->newRay(newRay, Event::Refract);
			}
		}

		void doDiffuseReflect(Real reflect) {
			Vec3 rd = randomDiffuseRay(N);
			Ray newRay(inter, rd, reflect * ray.fAmp, ray);
			newRay.fP = randomNorm(rd);
			this->newRay(newRay, Event::Reflect);
		};

		void doDiffuseTrans(Real trans) {
			Vec3 rd = randomDiffuseRay(-N);
			Ray newRay(inter, rd, trans * ray.fAmp, ray);
			newRay.fP = randomNorm(rd);
			this->newRay(newRay, Event::Refract);
		};

		void doExpReflect(Real reflect) {
			Vec3 rd = randomMetalRay(ray.fD, N, 0.15);
			Ray newRay(inter, rd, reflect * ray.fAmp, ray);
			newRay.fP = randomNorm(rd);
			this->newRay(newRay, Event::Reflect);
		};

		void doExpRefract(Real trans) {
			Vec3 td;
			if (transDirection(N, ray.fD,
				fromIndex, toIndex, td)) {
				Vec3 rd = randomMetalRay(td, -N, 0.15);
				Ray newRay(inter, rd, trans * ray.fAmp, ray);
				newRay.fP = randomNorm(rd);
				this->newRay(newRay, Event::Refract);
			}
		};

		Vec3 randomRayleigh(Vec3 n) {
			// pdf = 3/8 (1 + x^2); x = cos(theta)
			// cdf = 3/8 (x + 1/3 x^3) + 1/2
			// x + 1/3 x^3 = 8/3 (cdf - 1/2) = A

			Real cdf = uniform(0, 1);

#if 0
			Real const A = (8. / 3) * (cdf - 0.5);
			// x0 + 1/3 x0^3 = A0
			// x0 + dx + 1/3 x0^3 + x0^2 dx ~ A0 + (A - A0)
			// dx + x0^2 dx ~ A - A0
			// x = x0 + (A-A0)/(1 + x0^2)
			Real x = 0;
			for (int i = 0; i < 5; ++i) {
				Real x0 = x;
				Real A0 = x0 + (1 / 3.) * x0 * x0 * x0;
				x = x0 + (A - A0) / (1 + x0 * x0);
			}
#else
			Real const A = 4 * (cdf - 0.5);
			Real t = pow(sqrt(A * A + 1.) + A, 1. / 3.);
			Real x = t - 1 / t;
#endif

			if (x > 1) x = 1;
			else if (x < -1) x = -1;

			return n * x + sqrt(1 - x * x) * randomNorm(n);
		}

		void doRayleighReflect(Real reflect) {
			Vec3 rd = randomRayleigh(ray.fD);
			Ray newRay(inter, rd, reflect * ray.fAmp, ray);
			newRay.fP = randomNorm(rd);
			this->newRay(newRay, Event::Reflect);
		}

		void process() {
			auto& handler = rt.handler;
			auto& opts = rt.opts;

			SurfaceProperties const* sp = handler.property;
			Real reflect;
			Real refract;
			Real lambda = ray.fLambda;
			ReflectType reflectType;

			if (handler.inner) {
				N = -handler.N;
				reflectType = sp->fInnerReflectType;
				fromIndex = sp->fIndexInner.get(lambda);
				toIndex = sp->fIndexOuter.get(lambda);
				if (reflectType != ReflectType::Optical) {
					reflect = sp->fIn2OutReflect.ratio(inter, lambda);
					refract = sp->fIn2OutTrans.ratio(inter, lambda);
				}
			} else {
				N = handler.N;
				reflectType = sp->fOuterReflectType;
				fromIndex = sp->fIndexOuter.get(lambda);
				toIndex = sp->fIndexInner.get(lambda);
				if (reflectType != ReflectType::Optical) {
					reflect = sp->fOut2InReflect.ratio(inter, lambda);
					refract = sp->fOut2InTrans.ratio(inter, lambda);
				}
			}

			if (reflectType == ReflectType::Optical) {

				Vec3 pr;
				Vec3 pt;

				Vec3 ns = normalize(cross(ray.fD, N));
				Vec3 np = normalize(cross(ns, ray.fD));
				Real A_s = dot(ns, ray.fP);
				Real A_p = dot(np, ray.fP);

				if (checkRefractDirection(N, ray.fD, fromIndex, toIndex)) {

					Real n1 = fromIndex;
					Real n2 = toIndex;
					Real costheta1 = -dot(ray.fD, N);
					Real sintheta1 = sqrt(1 - Sqr(costheta1));
					Real sintheta2 = sintheta1 * n1 / n2;
					Real costheta2 = sqrt(1 - Sqr(sintheta2));

					Real rs = (n1 * costheta1 - n2 * costheta2) /
						(n1 * costheta1 + n2 * costheta2);
					Real ts = rs + 1.;

					Real rp = (n2 * costheta1 - n1 * costheta2) /
						(n2 * costheta1 + n1 * costheta2);
					Real tp = (rp + 1) * n1 / n2;



					Real R = Sqr(A_s * rs) + Sqr(A_p * rp);
					Real T = n2 * costheta2 / (n1 * costheta1)
						* (Sqr(A_s * ts) + Sqr(A_p * tp));

					pr = A_s * rs * ns + A_p * rp * cross(ns, reflectDirection(N, ray.fD));
					Vec3 np_out;
					transDirection(N, ray.fD, n1, n2, np_out);
					pt = A_s * ts * ns + A_p * tp * cross(ns, np_out);


					//Real R_ = norm2(pr);
					//Real T_ = n2 * costheta2 / (n1 * costheta1) * norm2(pt);
					pt = normalize(pt);
					pr = normalize(pr);

					reflect = R;
					refract = T;
				} else {
					reflect = 1;
					refract = 0;
					pr = A_s * ns + A_p * cross(ns, reflectDirection(N, ray.fD));
					pr = normalize(pr);
				}


				if (the_level < 1) {
					if (reflect > 0) {
						doMirrorReflect(reflect, pr);
					}
					if (refract > 0) {
						doMirrorTrans(refract, pt);
					}
				} else {
					if (reflect > 0 && uniform(0, 1) < reflect) {
						doMirrorReflect(1., pr);
					}
					if (refract > 0 && uniform(0, 1) < refract) {
						doMirrorTrans(1., pt);
					}
				}
			} else if (reflectType == ReflectType::Mirror) {

				if (the_level < 1) {
					if (reflect > 0) {
						doMirrorReflect(reflect);
					}
					if (refract > 0) {
						doMirrorTrans(refract);
					}
				} else {
					if (reflect > 0 && uniform(0, 1) < reflect) {
						doMirrorReflect(1.);
					}
					if (refract > 0 && uniform(0, 1) < refract) {
						doMirrorTrans(1.);
					}
				}

			} else if (reflectType == ReflectType::Diffuse) {

				if (the_level < 1) {
					if (reflect > 0) {
						for (int i = 0; i < opts.first_level_split; ++i) {
							doDiffuseReflect(reflect / opts.first_level_split);
						}
					}
					if (refract > 0) {
						for (int i = 0; i < opts.first_level_split; ++i) {
							doDiffuseTrans(refract / opts.first_level_split);
						}
					}
				} else {

					if (reflect > 0 && uniform(0, 1) < reflect) {
						doDiffuseReflect(1.);
					}
					if (refract > 0 && uniform(0, 1) < refract) {
						doDiffuseTrans(1.);
					}
				}
			} else if (reflectType == ReflectType::Metal) {

				if (the_level < 1) {
					if (reflect > 0) {
						for (int i = 0; i < opts.first_level_split; ++i) {
							doExpReflect(reflect / opts.first_level_split);
						}
					}
					if (refract > 0) {
						for (int i = 0; i < opts.first_level_split; ++i) {
							doExpRefract(refract / opts.first_level_split);
						}
					}

				} else {
					if (reflect > 0 && uniform(0, 1) < reflect) {
						doExpReflect(1.);
					}
					if (refract > 0 && uniform(0, 1) < refract) {
						doExpRefract(1.);
					}

				}
			} else if (reflectType == ReflectType::Rayleigh) {

				if (reflect > 0 && uniform(0, 1) < reflect) {
					doRayleighReflect(1.);
				}


			}

			if (rt.recorder) {
				if (die) {
					rt.recorder(Event::Die, ray, the_level + 1, handler);
				}
			}

			Real amp_ = sp->fBrightness;
			if (amp_) {
				rt.pixelAmp += amp_ * ray.fAmp;
			}
		}
	};

	void RayTracing::traceRay(Ray const& ray) {
		pixelAmp = 0.;
		frames.emplace_back(ray, 0);
		if (recorder)
			recorder(Event::Generate, ray, 0, handler);

		for (; !frames.empty();) {

			Frame frame = frames.back();
			frames.pop_back();

			Ray const& ray = frame.ray;
			int ray_level = frame.level;

			if (!(ray_level <= opts.max_level && ray.fAmp >= opts.min_ray_amp)) {
				continue;
			} else {
				handler.process = false;

				(void)emitRay(devs, ray, handler);

				if (!handler.process) {
					if (recorder)
						recorder(Event::Escape, ray, ray_level + 1, handler);
					continue;
				}

				ProcessReflection processReflection(*this,
					ray, ray_level);
				processReflection.process();
			}

		}

		if (recorder)
			recorder(Event::End, ray, 0, handler);

	}

	struct SourceSelecter : Source {

		std::vector<Real> as;
		std::vector<Source*>* srcs = nullptr;

		SourceSelecter() : Source(0) {}

		void initialize(std::vector<Source*>& src) {
			srcs = &src;
			as.resize(src.size());
			fAmp = 0;
			for (int i = 0; i < (int)src.size(); ++i) {
				fAmp += src[i]->fAmp;
				as[i] = fAmp;
			}
		}

		Ray generate() override {
			return getSource()->generate();
		}

		// get a source with prob prop to light power
		Source* getSource() {
			Real t = uniform(0, 1) * fAmp;
			auto p = std::lower_bound(as.begin(), as.end(), t);
			if (p == as.end()) {
				p = std::prev(p);
			}
			size_t idx = p - as.begin();
			return (*srcs)[idx];
		}
	};

	void Engine::doEmit(int N, Source& src) {
		RayTracing rt(fDevices, fRecorders);

		for (int n = 0; n < N; ++n) {
			Ray ray = src.generate();
			ray.fID = n;
			rt.handler.record = true;
			rt.traceRay(ray);
		}
	}

	struct SingleRaySource : Source {

		SingleRaySource(Ray ray) : Source(1) {
			this->ray = ray;
		}
		Ray ray;
		Ray generate() override {
			return ray;
		}
	};

	void Engine::emit(Ray const& ray) {
		SingleRaySource srs(ray);
		doEmit(1, srs);
	}

	void Engine::emit(int N) {
		if (fSources.size() == 0) {
			throw std::logic_error("want to emit however we have no source!");
		}
		SourceSelecter selecter;
		selecter.initialize(fSources);
		doEmit(N, selecter);
	}

	Real transprent(Vec3 rayDirection,
		Vec3 lightOrigin,
		Vec3 inter,
		Vec3 N,
		Color color) {
		Real cosphi = fabs(dot(N, rayDirection));
		Real thickness_ratio = (1 + 1E-6) / (cosphi + 1E-6);
		Real trans = pow(1 - color.A(), thickness_ratio);
		return trans;

	}

	Color CalPictureColor(
		Vec3 rayDirection,
		Vec3 lightOrigin,
		Vec3 inter,
		Vec3 N,
		Color color,
		Real trans) {
		Real light = CosAngle(lightOrigin - inter, N);
		if (light < 0) {
			light = (-light) * (1 - trans);
		}

		Real cosphi = fabs(dot(N, rayDirection));
		Real thickness_ratio = (1 + 1E-6) / (cosphi + 1E-6);
		Real alpha = 1 - pow(1 - color.A(), thickness_ratio);

		Color c = { color.R() * light,
			color.G() * light,
			color.B() * light,
			alpha,
		};
		return c;
	}


	double lighting(std::vector<Device*>& fDevices,
		Vec3 inter,
		Vec3 const& light,
		Ray const& ray) {
		Real trans = 1;

		TracingHandler ph;

		Vec3 d = normalize(light - inter);
		Ray testRay(inter, d, 1, ray);
		for (;;) {


			Real smin;
			Device* dev = minSDevice(fDevices, testRay, smin);
			if (!dev) {
				break;
			}
			if (smin >= sqrt(norm2(inter - light))) {
				break;
			}

			dev->process(testRay, ph);

			if (ph.process) {
				Color surfaceColor = !ph.inner ?
					ph.property->fInnerPictureColor :
					ph.property->fOuterPictureColor;
				inter = ph.inter;
				Real a = transprent(d, light, ph.inter, ph.N, surfaceColor);
				trans *= a;
				testRay.fO = inter;
			}

			if (trans == 0) {
				break;
			}
		}
		return trans;
	}


	Real scosAngle(Vec3 v, Vec3 const& n) {
		bool a = std::isinf(v.fX);
		bool b = std::isinf(v.fY);
		bool c = std::isinf(v.fZ);
		if (a || b || c) {
			if (a && !b && !c) {
				v = { 1,0,0 };
			} else if (!a && b && !c) {
				v = { 0,1,0 };
			} else if (!a && !b && c) {
				v = { 0,0,1 };
			}
		}
		return CosAngle(v, n);
	}

	Color CalPictureColor2(
		Vec3 rayDirection,
		Vec3 lightOrigin,
		Vec3 inter,
		Vec3 N,
		Color color) {
		Real light = Sqr((scosAngle(lightOrigin - inter, N) + 1) / 2);

		Real alpha;
		if (color.A() == 1.) {
			alpha = 1.;
		} else {
			Real cosphi = fabs(dot(N, rayDirection));
			Real thickness_ratio = (1 + 1E-6) / (cosphi + 1E-6);
			alpha = 1 - pow(1 - color.A(), thickness_ratio);
		}

		Color c = { color.R() * light,
			color.G() * light,
			color.B() * light,
			alpha,
		};
		return c;
	}

	Color pictureColor(Ray& ray,
		std::vector<Device*>& fDevices,
		TracingHandler& ph,
		PictureOpts const& opts) {
		Color color = { 0,0,0,0 };
		for (int i = 0; i < 999; ++i) {

			ph.process = false;
			(void)emitRay(fDevices, ray, ph);

			if (ph.process) {
				ph.process = false;

				Color surfaceColor = ph.inner ?
					ph.property->fInnerPictureColor :
					ph.property->fOuterPictureColor;

				Real alpha = ph.inner ?
					ph.property->fOuterPictureColor.A() :
					ph.property->fInnerPictureColor.A();

				Vec3 n = ph.inner ? -ph.N : ph.N;

				Color c;
				if (false) {
					// has shadow
					Real light;
					light = lighting(fDevices, ph.inter,
						opts.LightOrigin, ray);
					c = CalPictureColor(ray.fD,
						opts.LightOrigin,
						ph.inter,
						n,
						surfaceColor,
						alpha
					);
					c = c * light;
				} else {
					c = CalPictureColor2(ray.fD,
						opts.LightOrigin,
						ph.inter,
						n,
						surfaceColor
					);
				}


				c.precompute();

				color.precomputeOver(c);
				if (color.A() >= 0.99999) {
					break;
				}

				// continue
				ray.fO = ph.inter;
			} else {
				break;
			}
		}

		// black background
		color.precomputeOver(Color::black(1.));

		return color;
	}


	void Engine::devicesPicture(Bitmap& bmp,
		PictureOpts const& opts) {
		bmp.resize(opts.Width, opts.High);
		int w = opts.Width, h = opts.High;

		Vec3 n1 = normalize(opts.N1);
		Vec3 n2 = normalize(opts.N2);
		Vec3 d = -normalize(cross(n1, n2));

		Scaler s;
		s.xMin = opts.N1Min;
		s.xMax = opts.N1Max;
		s.yMin = opts.N2Min;
		s.yMax = opts.N2Max;
		s.w = opts.Width;
		s.h = opts.High;

		TracingHandler ph;

		for (int j = 0; j < h; ++j) {
			for (int i = 0; i < w; ++i) {

				Color color = Color::black(0.);
				int n = opts.AntiAliasLevel + 1;

				for (int ii = 0; ii < n; ++ii) {
					for (int jj = 0; jj < n; ++jj) {
						Real x = s.pixel2WorldX(i + (ii - 0.5 * (n - 1)) / n);
						Real y = s.pixel2WorldY(j + (jj - 0.5 * (n - 1)) / n);
						Ray ray;
						ray.fAmp = 1.;
						ray.fD = normalize(d + n1 * x + n2 * y);
						ray.fO = opts.Origin;
						ray.fLambda = 500;
						ray.fP = Vec3{};

						color += pictureColor(ray, fDevices, ph, opts);
					}
				}

				color /= Sqr(n);
				bmp.at(j, i) = color;

			}
		}

	}

	Bitmap Engine::devicesPicture(PictureOpts const& opts) {
		Bitmap bmp;
		devicesPicture(bmp, opts);
		return bmp; // move by default, no std::move is needed
	}


	struct PictrueJob {
		std::vector<std::thread> threads;
		std::atomic_int32_t line{ 0 };


		template<class T>
		void allocate_threads(Bitmap& bmp,
			std::function<T(int jobID)> constructor,
			std::function<void(T& rt, int hstart, int hend, int jobID)> callback,
			bool print_new_line = false)
		{
			int total = std::thread::hardware_concurrency();
			size_t w = bmp.fW;
			size_t h = bmp.fH;

			for (int jobID = 0; jobID < total; ++jobID) {

				std::thread t = std::thread(
					[rt = constructor(jobID), w, h, jobID, total, callback, this, print_new_line]()
					mutable {


					for (;;) {
						int32_t hstart = line.fetch_add(1);
						if (hstart >= h) {
							break;
						}
						if (print_new_line) {
							printf("processing line %3d\n", hstart);
						}
						callback(rt, hstart, hstart + 1, jobID);
					}
				}
				);
				threads.push_back(std::move(t));
			}
		}

		void join() {
			for (auto& t : threads) {
				t.join();
			}
		}
	};

	void Engine::devicesPicture(std::string const& filename,
		PictureOpts const& opts) {
		Bitmap bmp;
		devicesPicture(bmp, opts);
		bmp.write(filename);
	}

	void eye2(Bitmap& bmp,
		RayTracing& rt,
		int hstart,
		int hend,
		PictureOpts const& opts) {
		int PPP = opts.SamplePoints;
		size_t w = (size_t)opts.Width, h = (size_t)opts.High;

		Vec3 n1 = normalize(opts.N1);
		Vec3 n2 = normalize(opts.N2);

		// center-of-subject - center-of-camera
		Vec3 d = -normalize(cross(n1, n2));
		Scaler s;
		s.xMin = opts.N1Min;
		s.xMax = opts.N1Max;
		s.yMin = opts.N2Min;
		s.yMax = opts.N2Max;
		s.w = opts.Width;
		s.h = opts.High;
		int64_t cnt = 0;


		for (int j = hstart; j < hend; ++j) {
			for (int i = 0; i < w; ++i) {


				Color total = Color::black(0.);
				for (int ppp = 0; ppp < PPP; ++ppp) {

					Real ir = i + uniform(0, 1.);
					Real jr = j + uniform(0, 1.);
					// field of view, in rad
					Real x = s.pixel2WorldX(ir);
					Real y = s.pixel2WorldY(jr);


					Ray ray;
					ray.fID = cnt++;
					ray.fAmp = 1.;


					// point-on-camera
					Vec3 pc{};
					Vec3 rayD{};

					if (opts.ApertureDiameter == 0) {
						pc = opts.Origin;
						rayD = normalize(n1 * x + n2 * y + d);
					} else {
						Real rho = sqrt(uniform(0, 1)) * opts.ApertureDiameter;
						Real rhox;
						Real rhoy;
						randomSinCos(rhox, rhoy);
						rhox *= rho;
						rhoy *= rho;

						// point-on-camera - center-of-camera
						Vec3 pc_c = n1 * rhox + n2 * rhoy;
						pc = n1 * rhox + n2 * rhoy + opts.Origin;
						rayD = normalize(n1 * x + n2 * y + d
							-  (1./opts.FocalDistance) * pc_c);
					}


					ray.fD = rayD;
					ray.fP = randomNorm(rayD);
					ray.fO = pc;
					ray.fLambda = uniform(LEN_MIN, LEN_MAX);

					rt.traceRay(ray);

					Color c = Color::black(0.);
					WaveLength2RGB(ray.fLambda, &c.R(), &c.G(), &c.B());
					total += c * rt.pixelAmp;
				}
				if (PPP > 0) {
					total.cmul(1. / PPP);
				}
				total.A() = 1.;
				bmp.at(j, i) = total;
			}
		}
	}


	Bitmap Engine::eye(PictureOpts const& opts) {

		Bitmap bmp;
		bmp.resize(opts.Width, opts.High);

		if (!opts.Mult) {
			RayTracing rt(fDevices, fRecorders);
			eye2(bmp, rt, 0, opts.High, opts);
		} else {
			PictrueJob pj;

			auto constructor = [this](int) {
				return RayTracing(fDevices, fRecorders);
			};
			auto job = [&bmp, &opts, this](RayTracing& rt, int hstart, int hend, int index) {
				eye2(bmp, rt, hstart, hend, opts);
			};
			pj.allocate_threads<RayTracing>(bmp, constructor, job, opts.stdoutProgress);
			pj.join();
		}

		return bmp;
	}


	Device* Engine::findDevice(std::string_view name) {
		auto it = std::find_if(fDevices.begin(), fDevices.end(), [&](Device* d) {
			return d->getName() == name;
			});
		if (it == fDevices.end()) {
			return nullptr;
		}
		return *it;

	}

}

