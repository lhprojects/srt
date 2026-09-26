// blue sky and sunset from atmospheric scattering
#include "common.h"

void blueSky(int q)
{

    constexpr Real atmosphereThick = 1;
	auto earth = quadricSurface(
		pars::name = "earth",
        pars::shape = ShapeType::Shpere,
		pars::origin = Vec3{ 0,0,-5 },
		pars::radius = 5,
		pars::out2InRefractRatio = 0.0,
		pars::outerReflectRatio = 1.0,
		pars::outerReflectType = ReflectType::Diffuse);



    struct Atmosphere : Device
    {

        Real step = 0.2;
        Real Length = 0.5;
        std::shared_ptr<Surface> atmosphereTop;
        SurfaceProperties scattering_property;
        SurfaceProperties trans_property;

        Atmosphere()
        {
            setName("air");
            // the scattering length goes with the wavelength
            fWavelength = WavelengthUse::Tracing;
            atmosphereTop = quadricSurface(
                pars::name = "atmoTop",
                pars::shape = ShapeType::Shpere,
                pars::origin = Vec3{ 0,0,-5 },
                pars::radius = 5 + atmosphereThick,
                pars::in2OutRefractRatio = 1,
                pars::out2InRefractRatio = 1,
                pars::innerReflectRatio = 0,
                pars::outerReflectRatio = 0,
                pars::innerReflectType = ReflectType::Mirror,
                pars::outerReflectType = ReflectType::Mirror);

            scattering_property.fIn2OutReflect = 0.5;
            scattering_property.fIn2OutTrans = 0.5;
            scattering_property.fInnerReflectType = ReflectType::Rayleigh;
            trans_property.fIn2OutReflect = 0;
            trans_property.fIn2OutTrans = 1;
            trans_property.fInnerReflectType = ReflectType::Mirror;
        }

        bool intersect(Ray const& in, Real tMax, Hit& hit) const override {
            if (atmosphereTop->isInner(in.fO)) {
                // a step through the air
                if (step >= tMax) {
                    return false;
                }
                hit.t = step;
                hit.in2out = true;
                return true;
            }
            if (!atmosphereTop->intersect(in, tMax, hit)) {
                return false;
            }
            // the top of the atmosphere shades its own hit
            hit.sub = atmosphereTop.get();
            return true;
        }

        // only for a step through the air (see intersect)
        void shade(Ray const& in, Hit const& hit, TracingHandler& out) const override {
            Real len = Length / Sqr(Sqr(500 / in.fLambda));
            if (uniform(0, 1) < step / len) {
                out.hitSurface(in.fO + in.fD * step, in.fD,
                    true, &scattering_property, this);
            } else {
                out.hitSurface(in.fO + in.fD * step, in.fD,
                    true, &trans_property, this);
            }
        }

    };

    Engine en;

	auto sun = quadricSurface(pars::shape = ShapeType::Shpere,
        pars::outerReflectRatio = 0,
		pars::origin = Vec3{ 10,0,0 },
		pars::radius = 1,
		pars::brightness = 1);

    en.addDevice(earth);
    en.addDevice(std::make_shared<Atmosphere>());
    en.addDevice(sun);
    //en.addRecorder(logger());

	for (Real l = 0.5; l <= 2; l *= 4) {
        static_cast<Atmosphere*>(en.findDevice("air"))->Length = l;
        {
            sun->set(
                pars::shape = ShapeType::Shpere,
                pars::origin = Vec3{ 10,0,0 },
                pars::radius = 1);
            Bitmap bmp;
            if (q == kFAST) {
                bmp = en.eye(
                    PictureOpts(
                        pars::width = 100,
                        pars::high = 100,
                        pars::samplePerPixel = 100,
                        pars::origin = Vec3{ 0,0,0.05 },
                        pars::lookAt = Vec3{ 10,0,0 },
                        //pars::n1 = Vec3{ 0,1,0 },
                        //pars::n2 = Vec3{ 0,0,1 },
                        pars::fieldOfView = 2,
                        pars::mult = false
                    ));

            } else {
                bmp = en.eye(
                    PictureOpts(
                        pars::width = 200,
                        pars::high = 200,
                        pars::samplePerPixel = 5000,
                        pars::origin = Vec3{ 0,0,0.05 },
                        pars::lookAt = Vec3{ 10,0,0 },
                        //pars::n1 = Vec3{ 0,1,0 },
                        //pars::n2 = Vec3{ 0,0,1 },
                        pars::fieldOfView = 2,
                        pars::mult = true,
                        pars::stdoutProgress = false
                    ));
            }
            bmp.cnormalize();
            bmp.cclip(0.05); // make the sky more bright
            bmp.cnormalize();

            bmp.write("output/sunfail_l" + std::to_string(l) + ".png");

        }

        {
            sun->set(
                pars::shape = ShapeType::Shpere,
                pars::origin = Vec3{ 0,0,10 },
                pars::radius = 1);
            Bitmap bmp;
            if (q == kFAST) {
                bmp = en.eye(
                    PictureOpts(
                        pars::width = 100,
                        pars::high = 100,
                        pars::samplePerPixel = 100,
                        pars::origin = Vec3{ 0,0,0.05 },
                        pars::n1 = Vec3{ 1,0,0 },
                        pars::n2 = Vec3{ 0,1,0 },
                        pars::fieldOfView = 2,
                        pars::mult = false
                    ));

            } else if(q == kGOOD) {
                bmp = en.eye(
                    PictureOpts(
                        pars::width = 200,
                        pars::high = 200,
                        pars::samplePerPixel = 5000,
                        pars::origin = Vec3{ 0,0,0.05 },
                        pars::n1 = Vec3{ 0,1,0 },
                        pars::n2 = Vec3{ 1,0,0 },
                        pars::fieldOfView = 2,
                        pars::mult = true,
                        pars::stdoutProgress=false
                    ));
            } else {
				bmp = en.eye(
					PictureOpts(
						pars::width = 200,
						pars::high = 200,
						pars::samplePerPixel = 10*5000,
						pars::origin = Vec3{ 0,0,0.05 },
						pars::n1 = Vec3{ 0,1,0 },
						pars::n2 = Vec3{ 1,0,0 },
						pars::fieldOfView = 2,
						pars::mult = true,
						pars::stdoutProgress = true
					));
        }
            bmp.cnormalize();
            bmp.cclip(0.05); // make the sky more bright
            bmp.cnormalize();
            bmp.write("output/sunblue_l" + std::to_string(l) + ".png");
        }
    }

}

int gQuality = kGOOD;

// usage: sky [--threads N] [fast|good|best]    (default: good)
int main(int argc, char* argv[])
{
    // handles --threads; what is left is the quality, not example names
    init_examples(argc, argv);
    std::string q = gSelected.empty() ? "good" : gSelected[0];
    bool extra = gSelected.size() > 1;
    gSelected.clear();

    if (q == "fast" && !extra) {
        gQuality = kFAST;
    } else if (q == "good" && !extra) {
        gQuality = kGOOD;
    } else if (q == "best" && !extra) {
        gQuality = kBEST;
    } else {
        fprintf(stderr, "usage: sky [--threads N] [fast|good|best]\n");
        return 1;
    }

    printf("quality: %s\n", q.c_str());
    run(blueSky(gQuality));

    return 0;
}
