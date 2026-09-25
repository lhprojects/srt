// Newtonian telescope and plane stops
#include "common.h"

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

int main(int argc, char* argv[])
{
    init_examples(argc, argv);

    run(newtainTelescope(kFAST));
    run(testPlaneStop());

    return 0;
}
