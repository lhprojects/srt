#include "../srt/srt.h"
using namespace srt;

#include <memory>
#include <utility>
#include <fstream>
#include <chrono>
#include <filesystem>


int const kFAST = 0;
int const kGOOD = 1;
int const kBEST = 2;

// width=height=depth=1
std::shared_ptr<Surface> boxSurface() {
    auto cc = convex({});
    for (Real i = -1; i <= 1; ++i) {
        for (Real j = -1; j <= 1; ++j) {
            for (Real k = -1; k <= 1; ++k) {
                if (fabs(i) + fabs(j) + fabs(k) == 1) {
                    auto s1 = planeSurface(
                        pars::origin = Vec3{ 0.5 * i, 0.5 * j, 1. + 0.5 * k },
                        pars::direction = Vec3{ i,j, k },
                        pars::pictureColor = Color::white(1.));
                    cc->addSurface(s1);
                }
            }
        }
    }
    return cc;
}

void testParabola()
{

    Real d = 1E-3;
    Real qRadius = 2E-1;
    Real scn_wind = 1E-4;
    Real z = 1;
    Real s = 1.1;

    auto ps = planePositionSampler(
        pars::origin = Vec3{ 0,0, z },
        pars::n1 = Vec3{ 0,1,0 },
        pars::n2 = Vec3{ 1,0,d },
        pars::n1Min = -d * z - s * qRadius,
        pars::n1Max = -d * z + s * qRadius,
        pars::n2Min = -s * qRadius,
        pars::n2Max = +s * qRadius);

    auto ds = cosineDirectionSampler(0.00);
    
    auto src = comSource(
        pars::amp = 1.,
        pars::spectrum = monoSpectrum(600.),
        pars::directionSampler = ds,
        pars::positionSampler = ps);

    auto tube = quadricBound(
        pars::shape = ShapeType::Tube,
        pars::origin = Vec3{ 0,0,0 },
        pars::direction = Vec3{ 0,0,1 },
        pars::radius = qRadius);

    auto qs = quadricSurface(
        pars::name = "main mirror",
        pars::innerPictureColor = Color::red(1.),
        pars::outerPictureColor = Color::blue(1.),
        pars::innerReflectType = ReflectType::Mirror,
        pars::innerReflectRatio = 1.,
        pars::in2OutRefractRatio = 0.,
        pars::outerReflectType = ReflectType::Diffuse,
        pars::outerReflectRatio = 0.,
        pars::out2InRefractRatio = 0.,
        pars::shape = ShapeType::Parabola,
        pars::origin = Vec3{ 0,0,0 },
        pars::direction = Vec3{ 0,0,1 },
        pars::radius = 1.,
        pars::bound = tube);
    
    auto scn = std::make_shared<PlaneScreen>(
        pars::name = "screen",
        pars::origin = Vec3{ 0,0,0.5 },
        pars::direction = Vec3{ 0,0,1 },
        pars::reflectRatio = 0.,
        pars::refractRatio = 1.,
        pars::outerReflectType = ReflectType::Mirror,
        pars::innerReflectType = ReflectType::Mirror,
        pars::pictureColor = Color::green(0.1),
        pars::recordOut2In = false,
        pars::bound = tube);

    auto floor = std::make_shared<PlaneScreen>(
        pars::name = "floor",
        pars::origin = Vec3{ 0,0,0 },
        pars::direction = Vec3{ 0,0,1 },
        pars::reflectType = ReflectType::Diffuse,
        pars::reflectRatio = 0.0,
        pars::pictureColor = Color::white(0.1)
    );

    Engine en;
    en.addSource(src);
    en.addDevice(qs);
    en.addDevice(scn);
    en.addDevice(floor);
    en.emit(10000);

    Bitmap scnbmp;
    scn->raster(scnbmp, ScreenOpts(
        pars::gray = true,
        pars::width = 500,
        pars::high = 500,
        pars::n1 = Vec3{ 1,0,0 },
        pars::n2 = Vec3{ 0,1,0 },
        pars::n1Min = d / 2 - scn_wind,
        pars::n1Max = d / 2 + scn_wind,
        pars::n2Min = -scn_wind,
        pars::n2Max = +scn_wind
    ));

    scnbmp.draw("(1E-3,1E-3)",
        TextPaint(pars::color=Color::white(0.2),
            pars::fontSize=15,
            pars::verticalAlign = VerticalAlign::Center,
            pars::horizentalAlign = HorizentalAlign::Center),
        200, 250);

    scnbmp.draw("1E-5",
        TextPaint(pars::color = Color::white(),
            pars::fontSize = 15,
            pars::verticalAlign = VerticalAlign::Center,
            pars::horizentalAlign = HorizentalAlign::Center),
        470, 470);
    // 1E-4 ~ 500
    scnbmp.draw(Color::white(), 470-25, 470+25, 485, 486);

    scnbmp.write("output/Par_screen.png");

    floor->raster("output/Par_floor.png", ScreenOpts(
        pars::gray = true,
        pars::width = 200,
        pars::high = 200,
        pars::n1 = Vec3{ 1,0,0 },
        pars::n2 = Vec3{ 0,1,0 },
        pars::screenSize = 1.
     ));

    en.devicesPicture("output/Par_Devices.png", PictureOpts(
        pars::width = 1000,
        pars::high = 1000,
        pars::origin = Vec3{ 3,0, 1 },
        pars::lookAt = Vec3{ 0,0,0 },
        pars::fieldOfView = 1.,
        pars::antiAliasLevel = 2,
        pars::lightOrigin = Vec3{ 0,0,2 }
    ));
}

void addRoom(Engine &en)
{

	auto light = planeSurface(
		pars::origin = Vec3{ 0,0,5 - 1E-4 },
		pars::direction = Vec3{ 0,0,1 },
		pars::name = "light");

    auto w1 = planeSurface(
        pars::origin = Vec3{ -2,0,0 },
        pars::direction = Vec3{ -1,0,0 },
        pars::name = "back");
    auto w2 = planeSurface(
        pars::origin = Vec3{ 0,-5,0 },
        pars::direction = Vec3{ 0,-1,0 },
        pars::name = "left");
    auto w3 = planeSurface(
        pars::origin = Vec3{ 0,+5,0 },
        pars::direction = Vec3{ 0,+1,0 },
        pars::name = "right");
    auto w4 = planeSurface(
        pars::origin = Vec3{ 0,0,5 },
        pars::direction = Vec3{ 0,0,-1 },
        pars::name = "roof");
    auto w5 = planeSurface(
        pars::origin = Vec3{ 0,0,0 },
        pars::direction = Vec3{ 0,0,1 },
        pars::name = "floor");

    for (auto& wall : { w1, w2, w3, w4, w5 }) {
        wall->setTrans(0);
        wall->setGridTexture(1);
        wall->setReflectType(ReflectType::Diffuse);
    }

    {
        light->setBound(boxBound(
            pars::x0 = -3,
            pars::x1 = 3,
            pars::y0 = -3,
            pars::y1 = 3));
    }

    light->setName("light");
    light->setBrightness(1);
    light->setTrans(0.);
    light->setReflect(0.);
    light->setInnerReflectType(ReflectType::Mirror);
    light->setOuterReflectType(ReflectType::Mirror);

    en.addDevice(std::move(w1));
    en.addDevice(std::move(w2));
    en.addDevice(std::move(w3));
    en.addDevice(std::move(w4));
    en.addDevice(std::move(w5));
    en.addDevice(std::move(light));

}

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

void addTelescopeToEngin(Engine& en)
{
    Real trans = 0.5;
    Real mainMirrorRadius = 1.5;
    Real shellRadius = 0.2;
    Real shellLength = 1;
    Real holdZ = 0.5;

    auto shellBox = boxBound(pars::z0 = 0, pars::z1 = shellLength);
    auto shellhole = boxBound(
        pars::x0 = 0, pars::x1 = 1,
        pars::y0 = -0.05, pars::y1 = 0.05,
        pars::z0 = holdZ - 0.05, pars::z1 = holdZ + 0.05
    );

    auto shellBound = all({ shellBox, inverse(shellhole) });

    auto shell = quadricSurface(
        pars::name = "shell",
        pars::shape = ShapeType::Tube,
        pars::origin = Vec3{ 0,0,0 },
        pars::direction = Vec3{ 0,0,1 },
        pars::radius = shellRadius,
        pars::reflectRatio = 0.5,
        pars::innerReflectType = ReflectType::Diffuse,
        pars::outerReflectType = ReflectType::Diffuse,
        pars::bound = shellBound
        );

    Real zOff = shellRadius * shellRadius / (2 * mainMirrorRadius);

    auto undersea = planeBound(
        pars::origin = Vec3{ 0,0,0 },
        pars::direction = Vec3{ 0,0,1 });

    auto mainMirror = quadricSurface(
        pars::name = "mainMirror",
        pars::shape = ShapeType::Parabola,
        pars::direction = Vec3{ 0,0,1 },
        pars::origin = Vec3{ 0,0, -zOff },
        pars::radius = mainMirrorRadius,
        pars::innerReflectType = ReflectType::Mirror,
        pars::outerReflectType = ReflectType::Diffuse,
        pars::bound = undersea
        );

    auto secondMirrorBound = boxBound(pars::x0 = -0.05,
        pars::x1 = 0.05,
        pars::y0 = -0.05,
        pars::y1 = 0.05);

    auto secondMirror = std::make_shared<PlaneSurface>(
        pars::name = "secondMirror",
        pars::innerReflectType = ReflectType::Diffuse,
        pars::innerReflectRatio = 0.0,
        pars::outerReflectType = ReflectType::Mirror,
        pars::outerReflectRatio= 1.0,
        pars::origin = Vec3{ 0,0, 0.5 },
        pars::direction = Vec3{ 1, 0, -1 },
        pars::bound = secondMirrorBound
        );

    auto floor = std::make_shared<PlaneSurface>(
        pars::origin = Vec3{ 0,0,-1 },
        pars::direction = Vec3{ 0,0,1 },
        pars::name = "floor",
        pars::reflectRatio = 0.5,
        pars::innerReflectType = ReflectType::Diffuse,
        pars::outerReflectType = ReflectType::Diffuse);

    Real scnX = mainMirrorRadius / 2 - zOff - holdZ;

    auto scn = std::make_shared<PlaneScreen>(
        pars::name = "screen",
        pars::origin = Vec3{ scnX, 0, holdZ },
        pars::direction = Vec3{ -1, 0, 0 },
        pars::bound = boxBound(
            pars::y0 = -0.03,
            pars::y1 = +0.03,
            pars::z0 = holdZ - 0.03,
            pars::z1 = holdZ + 0.03),
        pars::reflectRatio = 0.,
        pars::refractRatio = 0.
        );


    mainMirror->setInnerPictureColor(Color::red());
    mainMirror->setOuterPictureColor(Color::green());
    secondMirror->setInnerPictureColor(Color::red());
    secondMirror->setOuterPictureColor(Color::red());
    shell->setPictureColor(Color::white(trans));
    scn->setPictureColor(Color::green());
    secondMirror->setPictureColor(Color::green());
    secondMirror->setPictureAlpha(1.0);
    secondMirror->setInnerPictureAlpha(0);

    en.addDevice(std::move(shell));
    en.addDevice(std::move(floor));
    en.addDevice(std::move(mainMirror));
    en.addDevice(std::move(secondMirror));
    en.addDevice(std::move(scn));
}

void newtainTelescope(int q)
{

    auto cirule = quadricBound(
        pars::shape = ShapeType::Tube,
        pars::origin = Vec3{ 0,0,0 },
        pars::direction = Vec3{ 0,0,1 },
        pars::radius = 0.2);

    auto psStop = planeStop(
        pars::origin = Vec3{ 0,0,1 },
        pars::n1 = Vec3{ 1,0,0 },
        pars::n2 = Vec3{ 0,1,0 },
        pars::n1Min = -0.2,
        pars::n1Max = +0.2,
        pars::n2Min = -0.2,
        pars::n2Max = +0.2,
        pars::bound = cirule);


    Real objectZ = 1E7;
    Real w = 100;
    auto pps = planePositionSampler();
    {


        pps->setNorms({ 1,0,0 }, { 0,1,0 });
        pps->setNorm1Bounds(-w / sqrt(3) / 2, w / sqrt(3));
        pps->setNorm2Bounds(-w / 2, w / 2);
        pps->setOrigin({ 0,0, objectZ });

        Vec3 z{ 0,0,1 };
        Vec3 p1{ 1 / sqrt(3), 0, 0 };
        Vec3 p2{ -0.5 / sqrt(3), -0.5, 0 };
        Vec3 p3{ -0.5 / sqrt(3), +0.5, 0 };
        p1 = p1 * w;
        p2 = p2 * w;
        p3 = p3 * w;

        auto t1 = planeBound();
        auto t2 = planeBound();
        auto t3 = planeBound();
        t1->setOP(p1, cross(z, p2 - p1));
        t2->setOP(p2, cross(z, p3 - p2));
        t3->setOP(p3, cross(z, p1 - p3));
        auto triangle = all({ t1, t2, t3 });

        pps->setBound(triangle);
    };

    auto src = comSource(1.,
        monoSpectrum(0),
        pps,
        stopDirectionSampler(psStop,
            cosineDirectionSampler())
    );

    auto light_scn = planeScreen(
        pars::origin = Vec3{ 0,0,objectZ - 0.1 },
        pars::direction = Vec3{ 0,0,1 },
        pars::name = "light_scn",
        pars::reflectRatio = 0.,
        pars::refractRatio = 1.,
        pars::reflectType = ReflectType::Mirror,
        pars::recordIn2Out = false);

    Engine en;
    addTelescopeToEngin(en);
    en.addSource(src);
    en.addDevice(light_scn);
    en.emit(1*10000);
    
    {
        
        Bitmap devicesPictrue;
        en.devicesPicture(devicesPictrue, PictureOpts(
            pars::high = 1000,
            pars::width = 1000,
            pars::lightOrigin = Vec3{ 2, 2, 5 },
            pars::antiAliasLevel = 2,
            pars::fieldOfView = 0.7,
            pars::origin = Vec3{ 3, 1, 2 },
            pars::lookAt = Vec3{ 0,0,0.5 }
        ));

        Bitmap screenPictrue;
        Screen* scn = dynamic_cast<PlaneScreen*>(en.findDevice("screen"));

        scn->raster(screenPictrue,
            ScreenOpts(
                pars::gray = true,
                pars::high = 200,
                pars::width = 200,
                pars::n1 = Vec3{ 0,1,0 },
                pars::n2 = Vec3{ 0,0,1 },
                pars::origin = Vec3{ 0,0,0.5 },
                pars::screenSize = w / objectZ
            ));

        devicesPictrue.draw(screenPictrue, 0, 200, 400, 600);
        devicesPictrue.draw("screen", 
            TextPaint(pars::fontSize = 30.,
                pars::horizentalAlign = HorizentalAlign::Center,
                pars::verticalAlign = VerticalAlign::Bottom),
            100, 400);

        Bitmap srcPictrue;
        light_scn->raster(srcPictrue,
            ScreenOpts(pars::high = 200,
                pars::width = 200,
                pars::n1 = Vec3{ 0, 1, 0 },
                pars::n2 = Vec3{ -1,0, 0 },
                pars::origin = Vec3{ 0,0, objectZ },
                pars::screenSize = 2 * w,
                pars::gray = true));

        devicesPictrue.draw(srcPictrue, 400, 600, 0, 200);
        devicesPictrue.draw("source", 
            TextPaint(
                pars::fontSize=30,
                pars::horizentalAlign = HorizentalAlign::Center,
                pars::verticalAlign = VerticalAlign::Top),
            500, 200);
        devicesPictrue.write("output/telescope_picture.png");
    }

}

void testParabola();


void testWaveLengthToColor()
{


    Bitmap bitmap;
    bitmap.resize(1000, 100);

    for (int j = 0; j < 1000; ++j) {
        for (int i = 0; i < 100; ++i) {

            Real wavelength = LEN_MIN + (LEN_MAX - LEN_MIN) / 1000 * j;
            //Real wavelength = 450;
            Color c = Color::black(1);
            WaveLength2RGB(wavelength,
                &c.R(), &c.G(), &c.B());
            bitmap.at(i, j) = c;
        }
    }
    for (int j = 0; j < 1000; ++j) {
        Real wavelength = LEN_MIN + (LEN_MAX - LEN_MIN) / 1000 * j;
        if (j % 100 == 0) {
            bitmap.draw(Color::white(), j - 1., j + 1., 0, 20);
            char b[100];
            snprintf(b, 99, "%dum", int(wavelength));
            bitmap.draw(b, TextPaint(pars::fontSize=20), j + 3., 0);
        }
    }

    bitmap.write("output/WaveLengthToColor.png");


}
void testPlaneStop()
{
    PlaneStop ps;
    ps.fN1 = { 1,0,0 };
    ps.fN2 = { 0,1,0 };
    ps.fN1Min = 1000;
    ps.fN1Max = 1001;
    ps.fN2Min = 1000;
    ps.fN2Max = 1001;
    ps.fO = { 0,0,0 };
    Vec3 o = { 0,0,1 };

    std::vector<Ray> vec3;
    for (int i = 0; i < 1000000; ++i) {
        Vec3 inter = ps.sample(o);
        Ray r(inter, { 0,0,1 }, 1, 580, randomNorm({ 0,0,1 }), i);
        vec3.push_back(r);
    }
    Bitmap bitmap;
    ScreenOpts opts;
    opts.N1Min = 999;
    opts.N1Max = 1002;
    opts.N2Min = 999;
    opts.N2Max = 1002;

    auto rng = range(vec3);
    Raster().raster(bitmap, rng, opts);
    bitmap.write("output/TestStop.png");


}

#define nassert(x, y, t) if (fabs(x - y) > t){ printf("test failed:%d\n", __LINE__);}
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

void testPlankLaw()
{
    PlankLaw pl(5000);
    std::ofstream os("output/testPlankLaw.csv");

    Real N = 0;
    Real E = 0;
    Real s1 = 0;
    Real s2 = 0;
    Real s3 = 0;
    Real mx = 0;
    Real lst = 0;
    for (int i = 0; i < 100000; ++i) {
        Real x = PlankLaw::sampleX();
		E += 1;
		N += 1. / x;
		if (1 / x < 0.1006) {
			s1 += 1;
		}
        if (1 / x < 0.2855) {
            s2 += 1;
        }
        if (1 / x < 1.5905) {
            s3 += 1;
        }
        Real sp = pl.sample();
        if (sp > mx) {
            mx = sp;
        }

        os << sp << "," << 1 / sp << ",\n";
        lst = sp;
    }
    nassert(E / N, 2.7, 0.1);
    nassert(s1 / E / 0.01, 1, 0.1);
    nassert(s2 / E / 0.5, 1, 0.05);
	nassert(s3 / E / 0.99, 1, 0.001);
}

void testT2Color()
{
    Bitmap bitmap;
    bitmap.resize(1000, 100);

    for (int j = 0; j < 1000; ++j) {
        Real T = 10. * j + 10;
        for (int i = 0; i < 100; ++i) {


            PlankLaw s(T);
            Color t = Color::black(0);            
            for (Real Len = LEN_MIN; Len <= LEN_MAX; ++Len) {
                Color c = Color::black(1.);
                WaveLength2RGB(Len,
                    &c.R(), &c.G(), &c.B());
                t += c * s.pdf(Len);
            }

            t.A() = 1;

            bitmap.at(i, j) = t;
        }
    }

    for (int j = 0; j < 1000; ++j) {
        Real T = 10. * j + 10;
        if (j % 100 == 0) {
            bitmap.draw(Color::white(), j - 1., j + 1., 0, 20);
            char b[100];
            snprintf(b, 99, "%dK", int(T));
            bitmap.draw(b,
                TextPaint(pars::fontSize=20),
                j + 3., 0);
        }
    }

    bitmap.write("output/Temp2Color.png");
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

void dispersivePrism(int s)
{
    Real dist = 20;
    Engine en;

    auto build_triangle = [](){

        auto p1 = planeSurface(
            pars::origin = Vec3{ 0,0,1 },
            pars::direction = normalize(Vec3{ 0,1,0.1 }),
            pars::pictureColor = Color::white(0.1));

        auto p2 = planeSurface(
            pars::origin = Vec3{ 0,0,1 },
            pars::direction = normalize(Vec3{ 0,-1,0.1 }),
            pars::reflectType = ReflectType::Mirror,
            pars::pictureColor = Color::white(0.1));

        auto p3 = planeSurface(
            pars::origin = Vec3{ 0,0,-1 },
            pars::direction = normalize(Vec3{ 0,0,-1 }),
            pars::pictureColor = Color::white(0.1));

        auto p4 = planeSurface(
            pars::origin = Vec3{ 1,0,0 },
            pars::direction = normalize(Vec3{ 1,0, 0 }),
            pars::pictureColor = Color::red(0.9));

        auto p5 = planeSurface(
            pars::origin = Vec3{ -1,0,0 },
            pars::direction = normalize(Vec3{ -1,0, 0, }),
            pars::pictureColor = Color::red(0.9));

        std::vector<std::shared_ptr<PlaneSurface>> planes
            = { p1,p2,p3,p4,p5 };
        for (auto& s : planes) {
            s->set(
                pars::innerPredefinedSellmeier3 = PredefinedSellmeier3::BK7,
                pars::reflectType = ReflectType::Mirror,
                pars::innerReflectRatio = 0,
                pars::outerReflectRatio = 0,
                pars::in2OutRefractRatio = 1.,
                pars::out2InRefractRatio = 1.
            );
        }
        return convex({ p1,p2,p3,p4,p5 });
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

        void process(Ray const& in, ProcessHandler& handler) const {

                if (handler.fType == HandlerType::Distance) {
                    if (atmosphereTop->isInner(in.fO)) {
                        static_cast<DistanceHandler&>(handler).distance(step, true);
                    } else {
                        atmosphereTop->process(in, handler);
                    }
                } else if (handler.fType == HandlerType::Tracing) {
                    if (atmosphereTop->isInner(in.fO)) {
                        Real len = Length / Sqr(Sqr(500 / in.fLambda));
                        if (uniform(0, 1) < step / len) {
                            static_cast<TracingHandler&>(handler).hitSurface(
                                in.fO + in.fD * step, in.fD,
                                true, &scattering_property, this);
                        } else {
                            static_cast<TracingHandler&>(handler).hitSurface(
                                in.fO + in.fD * step, in.fD,
                                true, &trans_property, this);
                        }
                    } else {
                        atmosphereTop->process(in, handler);
                    }
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

void testLookAt() {
    PictureOpts opts;
    opts.Origin = Vec3{0,0,1};
    opts.lookAt(Vec3{ 0,0,0 });
}

void timeUniformNorm() {

    Real c = 0;
    for (int i = 0; i < 1000 * 1000; ++i) {
        Vec3 v = {0,0,1};
        c += norm2(randomNorm(v));
    }
    //printf("%f\n", c/(1000 * 1000));
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

void do_run(void (f)(), char const* s)
{
    printf("%30s ", s);
    auto t0 = std::chrono::high_resolution_clock::now();
    f();
    auto t1 = std::chrono::high_resolution_clock::now();
    using seconds = std::chrono::duration<double>;
    double d = seconds(t1 - t0).count();
    printf("%10fs\n", d);
}

#define run(f) do_run([](){ f; }, #f)
void real_main()
{
    std::filesystem::create_directory("./output");


    int q = kGOOD;
    run(testCone(kFAST));
    run(timeUniformNorm());
    run(testSphere(kFAST));
    run(testBoxSurface(kGOOD));
    run(testParabola());
    run(testLookAt());
    run(testUniform());
    run(testPlankLaw());
    run(testT2Color());
    run(source_distribution(0));
    run(source_distribution(1));
    run(dispersivePrism(kFAST));
    run(Glass(kFAST));
    run(blueSky(kGOOD));
    run(BoundDiagram());
    run(testSphereRefract());
    run(newtainTelescope(kFAST));
    run(testPlaneStop());
    run(testWaveLengthToColor());
    run(blueSky(kBEST));

}


int main() {
    real_main();
    return 0;
}
