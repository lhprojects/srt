// camera ray tracing: room scenes, materials, depth of field
#include "common.h"

void Glass(int q)
{
    Engine en;
    {

        auto qs3 = quadricSurface(
            pars::shape = ShapeType::Tube,
            pars::origin = Vec3{ 0, 0, 0 },
            pars::direction = Vec3{ 0,0,1 },
            pars::radius = 1.,
            pars::name = "barrel",
            pars::pictureColor = Color::red(0.5),
            pars::reflectType = ReflectType::Optical,
            pars::innerIndex = 1.5);

        auto ec1 = planeSurface(
            pars::origin = Vec3{ 0,0,1.5 },
            pars::direction = Vec3{ 0,0,1 },
            pars::name = "top",
            pars::pictureColor = Color::blue(0.5),
            pars::reflectType = ReflectType::Optical,
            pars::innerIndex = 1.5);

        auto ec2 = planeSurface(
            pars::origin = Vec3{ 0,0,0 },
            pars::direction = Vec3{ 0,0,-1 },
            pars::name = "bottom",
            pars::pictureColor = Color::blue(0.5),
            pars::reflectType = ReflectType::Optical,
            pars::innerIndex = 1.5);

        en.addDevice(convex({ ec1, ec2, qs3 }));
    }

    addRoom(en);

    {
        PictureOpts opts(
            pars::mult = true,
            pars::origin = Vec3{ 10,0, 3 },
            pars::lookAt = Vec3{ 0,0,2 },
            pars::fieldOfView = 1.2);

        if (q == kFAST) {
            opts.set(pars::width = 100,
                pars::high = 100,
                pars::samplePerPixel = 100);
            Bitmap bmp = en.eye(opts);
            bmp.cnormalize();
            bmp.write("output/tube_raytrace_fast.png");
        } else if (q == kGOOD) {
            opts.set(pars::width = 1000,
                pars::high = 1000,
                pars::samplePerPixel = 1000);
            Bitmap bmp = en.eye(opts);
            bmp.cnormalize();
            bmp.write("output/tube_raytrace_good.png");
        } else if (q == kBEST) {
            opts.set(pars::width = 2000,
                pars::high = 2000,
                pars::samplePerPixel = 10000);
            Bitmap bmp = en.eye(opts);
            bmp.cnormalize();
            bmp.write("output/tube_raytrace_best.png");
        }

    }


    auto tracker = tracking(0.01);
    en.addRecorder(tracker);

    Ray r({ 10,0,1 }, normalize({ -1,0.04,-0.1 }), 1., 0,
        randomNorm(normalize({ -1,0.04,-0.1 })),
        0);
    en.emit(r);

    en.addDevice(tracker);
    if (1) {

        PictureOpts opts(pars::width = 1000,
            pars::high = 1000,
            pars::antiAliasLevel = 3,
            pars::lightOrigin = Vec3{ 3,3,3 },
            pars::origin = Vec3{ 10,0,3 },
            pars::lookAt = Vec3{ 0,0,2 },
            pars::fieldOfView = 1.2);
        en.devicesPicture("output/tube_picture.png", opts);
    }

}

void testBoxSurface(int q) {
    Engine en;
    addRoom(en);
    en.addDevice(boxSurface());

    if(1) {
        PictureOpts opts(
            pars::mult = true,
            pars::origin = Vec3{ 2,0,2 },
            pars::lookAt = Vec3{ 0.5,0,2 },
            pars::fieldOfView = 1.,
            pars::apertureDiameter = 0.05
        );

        if (q == kFAST) {
            opts.set(pars::width = 100,
                pars::high = 100,
                pars::samplePerPixel = 100);
            Bitmap bmp = en.eye(opts);
            bmp.cnormalize();
            bmp.write("output/box_raytrace_fast.png");
        } else if (q == kGOOD) {
            opts.set(pars::width = 200,
                pars::high = 200,
                pars::samplePerPixel = 5000);
            Bitmap bmp = en.eye(opts);
            bmp.cnormalize();
            bmp.write("output/box_raytrace_good.png");
        } else if (q == kBEST) {
            opts.set(pars::width = 2000,
                pars::high = 2000,
                pars::samplePerPixel = 10000);
            Bitmap bmp = en.eye(opts);
            bmp.cnormalize();
            bmp.write("output/box_raytrace_best.png");
        }

    }
    if (1) {

        PictureOpts opts(pars::width = 1000,
            pars::high = 1000,
            pars::antiAliasLevel = 3,
            pars::lightOrigin = Vec3{ 3,3,3 },
            pars::origin = Vec3{ 2,0,2 },
            pars::lookAt = Vec3{ 0.5,0,2 },
            pars::fieldOfView = 1.);
        en.devicesPicture("output/box_picture.png", opts);
    }

}

void testSphere(int q)
{

    auto qs1 = quadricSurface(
        pars::name = "qs1",
        pars::shape = ShapeType::Shpere,
        pars::origin = Vec3{ 0, -3.6, 1 },
        pars::radius = 1.,
        pars::pictureColor = Color::red(1.),
        pars::reflectType = ReflectType::Diffuse);
    qs1->setOuterReflect(gaussSpectrum(1., WaveLengthRed, 25));

    auto qs2 = quadricSurface(
        pars::name = "qs2",
        pars::shape = ShapeType::Shpere,
        pars::origin = Vec3{ 0, -1.2, 1 },
        pars::radius = 1.,
        pars::pictureColor = Color::red(1.),
        pars::reflectType = ReflectType::Mirror,
        pars::reflectRatio = 1.,
        pars::refractRatio = 0.
    );

    auto qs3 = quadricSurface(
        pars::name = "qs3",
        pars::shape = ShapeType::Shpere,
        pars::origin = Vec3{ 0, 1.2, 1 },
        pars::radius = 1.,
        pars::pictureColor = Color::red(1.),
        pars::reflectType = ReflectType::Optical,
        pars::innerIndex = 1.5
    );

    auto qs4 = quadricSurface(
        pars::name = "qs4",
        pars::shape = ShapeType::Shpere,
        pars::origin = Vec3{ 0, 3.6, 1 },
        pars::radius = 1.,
        pars::pictureColor = Color::red(1.),
        pars::reflectType = ReflectType::Metal,
        pars::reflectRatio = 1.,
        pars::refractRatio = 0.
    );

    Engine en;
    addRoom(en);

    en.addDevice(qs1);
    en.addDevice(qs2);
    en.addDevice(qs3);
    en.addDevice(qs4);

    {
        PictureOpts opts(
            pars::width = 1000,
            pars::high = 1000,
            pars::antiAliasLevel = 2,
            pars::lightOrigin = Vec3{ 3,3, 3 },
            pars::origin = Vec3{ 10,0,2.5 },
            pars::lookAt = Vec3{ 0,0,0 },
            pars::fieldOfView = 1.2);
        en.devicesPicture("output/shpere_on_floor_picture.png", opts);
    }

    {
        PictureOpts opts(
            pars::mult = true,
            pars::lightOrigin = Vec3{ 3,3, 3 },
            pars::origin = Vec3{ 10,0,2.5 },
            pars::lookAt = Vec3{ 0,0,0 },
            pars::fieldOfView = 1.2);
        if (q == kFAST) {
            opts.set(
                pars::width = 100,
                pars::high = 100,
                pars::samplePerPixel = 100);
        } else if(q == kGOOD){
            opts.set(
                pars::width = 500,
                pars::high = 500,
                pars::samplePerPixel = 500);
        }
        Bitmap bmp = en.eye(opts);
        bmp.cnormalize();
        bmp.write("output/shpere_on_floor_raytrace.png");

    }
}

void testSphereRefract()
{
    // 1/u + n/v = (n-1)/R

    auto src = comSource(1,
		monoSpectrum(0),
        pointPositionSampler(Vec3{ 0,0,0 }, Vec3{0,0,1}),
		cosineDirectionSampler(0.1)
        );

    auto qs = std::make_shared<QuadricSurface>();
    qs->setSphere(Vec3{ 0,0,3 }, 1);
    qs->setInnerIndex(2);
    qs->setReflect(0.);
    qs->setTrans(1.);
    qs->setReflectType(ReflectType::Mirror);
    qs->setBound(planeBound(
        pars::origin = Vec3{ 0,0, 3 },
        pars::direction = Vec3{ 0,0, 1 }));

    auto scn = std::make_shared<PlaneScreen>(
        pars::origin = Vec3{ 0,0, 6 },
        pars::direction = Vec3{ 0,0,1 },
        pars::reflectRatio = 0.,
        pars::refractRatio = 0.);

    Engine en;
    en.addSource(src);
    en.addDevice(qs);
    en.addDevice(scn);
    en.emit(100000);

    ScreenOpts opts;
    opts.Gray = true;
    opts.N1 = { 1,0,0 };
    opts.N2 = { 0,1,0 };
    opts.N1Min = -5E-2;
    opts.N1Max = +5E-2;
    opts.N2Min = -5E-2;
    opts.N2Max = +5E-2;
    Bitmap bitmap;
    scn->raster(bitmap, opts);

    for (auto& c : bitmap.fC) {
        if (c.cmax() > 0) {
            c.cmul(pow(c.cmax(), 0.5 - 1));
        }
    }
    bitmap.write("output/testSphereRefract.png");

}

void testLookAt() {
    PictureOpts opts;
    opts.Origin = Vec3{0,0,1};
    opts.lookAt(Vec3{ 0,0,0 });
}

void testCone(int) {
    
    auto qs = quadricSurface(pars::shape = ShapeType::Cone,
        pars::radius = 1.,
        pars::direction = Vec3{0,0,1},
        pars::top_height = 2,
        pars::top_radius = 0.5);

    Engine en;
    en.addDevice(qs);
    addRoom(en);

    PictureOpts opts(
        pars::width = 1000,
        pars::high = 1000,
        pars::antiAliasLevel = 2,
        pars::lightOrigin = Vec3{ 3,3, 3 },
        pars::origin = Vec3{ 10,0,2.5 },
        pars::lookAt = Vec3{ 0,0,0 },
        pars::fieldOfView = 1.2);
    auto bmp = en.devicesPicture(opts);
    bmp.write("output/cone.png");

    
}

void testFormat(int) {
    using namespace srt;

    std::cout << std::format("{}", *quadricSurface(pars::shape = ShapeType::Shpere,
        pars::radius = 1.));

}

int main(int argc, char* argv[])
{
    init_examples(argc, argv);

    run(testCone(kFAST));
    run(testSphere(kFAST));
    run(testBoxSurface(kGOOD));
    run(testLookAt());
    run(Glass(kFAST));
    run(testSphereRefract());
    run(testFormat(kBEST));

    return 0;
}
