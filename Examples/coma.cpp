// coma of an off-axis parabolic mirror
#include "common.h"

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

int main(int argc, char* argv[])
{
    init_examples(argc, argv);

    run(testParabola());

    return 0;
}
