// dispersive prism
#include "common.h"

void dispersivePrism(int s)
{
    Real dist = 20;
    Engine en;

    auto build_triangle = [](){

        auto prism = std::make_shared<ConvexPolyhedron>();
        prism->addFace(Vec3{ 0,0,1 }, normalize(Vec3{ 0,1,0.1 }))
            .set(pars::pictureColor = Color::white(0.1));
        prism->addFace(Vec3{ 0,0,1 }, normalize(Vec3{ 0,-1,0.1 }))
            .set(pars::reflectType = ReflectType::Mirror,
                pars::pictureColor = Color::white(0.1));
        prism->addFace(Vec3{ 0,0,-1 }, normalize(Vec3{ 0,0,-1 }))
            .set(pars::pictureColor = Color::white(0.1));
        prism->addFace(Vec3{ 1,0,0 }, normalize(Vec3{ 1,0, 0 }))
            .set(pars::pictureColor = Color::red(0.9));
        prism->addFace(Vec3{ -1,0,0 }, normalize(Vec3{ -1,0, 0, }))
            .set(pars::pictureColor = Color::red(0.9));

        // every face
        prism->set(
            pars::innerPredefinedSellmeier3 = PredefinedSellmeier3::BK7,
            pars::reflectType = ReflectType::Mirror,
            pars::innerReflectRatio = 0,
            pars::outerReflectRatio = 0,
            pars::in2OutRefractRatio = 1.,
            pars::out2InRefractRatio = 1.
        );
        return prism;
    };

    auto tri = build_triangle();


    auto scn = planeScreen(
		pars::origin = Vec3{ 0,dist,0 },
		pars::direction = Vec3{ 0,1,0 },
		pars::bound = boxBound(pars::x0 = -1, pars::x1 = 1,
			pars::z0 = -1, pars::z1 = 1),
        pars::pictureColor = Color::white()
    );

    en.addDevice(tri);
    en.addDevice(scn);

    struct SampleInLine : PositionSampler {
        Vec3 sample(Vec3& norm) override {
			norm = Vec3{ 0,+1,0.06 }; // ray direction
			return Vec3{ uniform(-0.01,0.01),-4,0 };
        }
    };
    en.addSource(comSource(pars::amp = 1,
		pars::spectrum = plankSpectrum(pars::temperature = 5000),
		pars::positionSampler = std::make_shared<SampleInLine>(),
		pars::directionSampler = uniformDirectionSampler(0)
    ));

    if (kFAST == s) {
        en.emit(1000);
    }
    else {
        en.emit(1000000);
    }

    if (false) {
        auto tracker = tracking(0.01);
        en.addRecorder(tracker);
        en.addDevice(tracker);

        en.addRecorder(logger());

        en.emit(10);
    }
    
	scn->raster("output/prismColorSpectrum.png",
		ScreenOpts(pars::width = 200,
			pars::high = 200,
			pars::n1 = Vec3{ 1,0,0 },
			pars::n2 = Vec3{ 0,0,1 },
			pars::n1Min = -0.02,
			pars::n1Max = 0.02,
			pars::n2Min = -0.7,
			pars::n2Max = -0.6));

	auto bmp = en.devicesPicture(
		PictureOpts(
			pars::width = 2000,
			pars::high = 400,
			pars::lightOrigin = Vec3{ 5,dist / 2,5 },
			pars::origin = Vec3{ 40,dist / 2,0.5 },
			pars::lookAt = Vec3{ 0,dist / 2,0 },
			pars::antiAliasLevel = 2,
			pars::fieldOfView1 = 1,
            pars::fieldOfView2 = 0.2
            )
	);
	bmp.write("output/dispersivePrism.png");

}

int main(int argc, char* argv[])
{
    init_examples(argc, argv);

    run(dispersivePrism(kFAST));

    return 0;
}
