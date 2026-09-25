// random sampling checks: uniform directions, source distributions, bounds
#include "common.h"

void testUniform() {
    std::ofstream os("output/uniform.csv");

    Real s1 = 0;
    Real s2 = 0;
    Real s3 = 0;
    Real s4 = 0;
    int N = 10000;
    for (int i = 0; i < N; ++i) {
		s1 += uniform(0, 1);
		s2 += Sqr(uniform(0, 1));
		s3 += uniform(0, 1)* uniform(0, 1);
        // gamma distribution
        s4 += -log(uniform(0, 1) * uniform(0, 1) * uniform(0, 1) * uniform(0, 1));
        os << uniform(0, 1) << ",\n";
    }
    s1 /= N;
    s2 /= N;
    s3 /= N;
    s4 /= N;
    nassert(s1, 0.5, 0.05);
    nassert(s2, 0.333, 0.03);
    nassert(s3, 0.25, 0.02);
    nassert(s4, 4., 0.05);

    Real s5 = 0;
    Real s6 = 0;
    Real N2= 100*1000*1000;
    for (int i = 0; i < N2; ++i) {
        Real u5 = uniform(0, 1);
        Real u6 = uniform(0, 1);
        if (u5 < 1E-4) {
			s5 += 1 / sqrt(u5);
        }
        if (1-u5 < 1E-4) {
            s6 += 1 / sqrt(1-u5);
        }
    }
    s5 /= N2 * (2 * sqrt(1E-4));
	s6 /= N2 * (2 * sqrt(1E-4));

    //printf("%f\n", s5); // must closed 1
    //printf("%f\n", s6); // must closed 1

    Real s7 = 0;
    Real N3 = 1000 * 1000 * 1000;
    for (Real i = 0; i < N3; i+=1) {
        Real u5 = uniform(0, 1);
        Real u6 = uniform(0, 1);
        if (u5 < 1E-3 && u6 < 1E-3) {
            s7 += 1 / sqrt(u5) / sqrt(u6);
        }
    }
    s7 /= N3 * Sqr(2 * sqrt(1E-3));

    //printf("%f\n", s7); // must closed 1

}

void BoundDiagram()
{

    auto bb = boxBound(
        pars::x0 = -0.5,
        pars::x1 = +0.5,
        pars::y0 = -0.5,
        pars::y1 = +0.5,
        pars::z0 = +0.5,
        pars::z1 = +1.5);

    auto sphere = quadricSurface(
        pars::name = "1",
        pars::shape = ShapeType::Shpere,
        pars::origin = Vec3{ 0,0,0 },
        pars::radius = 1,
        pars::bound = bb,
        pars::pictureColor = Color::red(1.0)
    );

    auto sphere2 = std::make_shared<QuadricSurface>(*sphere);
    sphere2->setName("2");
    sphere2->setBound(inverse(bb));
    sphere2->setPictureColor(Color::red(0.2));

    auto floor = planeSurface(
        pars::origin = Vec3{ 0,0,-1 },
        pars::direction = Vec3{ 0,0,1 });


    Engine engine;

	engine.addDevice(boxSurface());
    engine.addDevice(sphere);
    engine.addDevice(sphere2);
    engine.addDevice(floor);
    Bitmap bmp;

    engine.emit(Ray({2,0,1.2},{-1,0,0},1,0,Vec3(),0));

    engine.devicesPicture(bmp, PictureOpts(
        pars::width = 1000,
        pars::high = 1000,
        pars::lightOrigin = Vec3{ 0, 0, 4 },
        pars::antiAliasLevel = 5,
        pars::fieldOfView = 1,
        pars::origin = Vec3{ 3, 1, 3 },
        pars::lookAt = Vec3{ 0, 0, 0.0 }
    ));

    bmp.write("output/BoundDiagram.png");

}

void source_distribution(int w)
{
    std::shared_ptr<Source> src;
    if (w == 0) {
        src = comSource(1,
            monoSpectrum(0),
            pointPositionSampler({ 0,0,0 }, { 0,0,1 }),
            uniformDirectionSampler());
    } else {
        src = comSource(1,
            monoSpectrum(0),
            pointPositionSampler({ 0,0,0 }, { 0,0,1 }),
            cosineDirectionSampler());
    }

    int N = 1000000;
    int bins = 200;
    std::vector <double> a(bins);
    for (int k = 0; k < N; ++k) {
        Ray r = src->generate();
        double cosine = r.fD.fZ;
		int i = (int)std::floor(0.5 * (cosine + 1) * bins);
        a.at(i) += 1;
    }

	printf("[");
	for (int i = 0; i < bins; ++i) {
		double v = a.at(i);
		double cosine = 1. * (i + 0.5 - bins / 2) / (bins / 2);
		if (w == 0) {
			double pdf = 0.5;
			printf("%.2f ", v / (pdf * 2 * N / bins));
		} else {
			double pdf = 2 * cosine;
			if (cosine > 0) {
				printf("%.2f ", v / (pdf * 2 * N / bins));
			}
		}
		++i;
    }
    printf("]\n");

}

void timeUniformNorm() {

    Real c = 0;
    for (int i = 0; i < 1000 * 1000; ++i) {
        Vec3 v = {0,0,1};
        c += norm2(randomNorm(v));
    }
    //printf("%f\n", c/(1000 * 1000));
}

int main(int argc, char* argv[])
{
    init_examples(argc, argv);

    run(timeUniformNorm());
    run(testUniform());
    run(source_distribution(0));
    run(source_distribution(1));
    run(BoundDiagram());

    return 0;
}
