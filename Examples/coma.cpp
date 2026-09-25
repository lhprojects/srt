// coma of an off-axis parabolic mirror
#include "common.h"

struct ComaScene
{
    std::shared_ptr<PlaneScreen> screen;
    std::shared_ptr<PlaneScreen> floor;
};

// parabolic mirror (focal length 0.5, aperture radius 0.2), a transparent
// screen in its focal plane and an absorbing floor, lit by a collimated
// 600nm beam tilted by d radians about the y axis
ComaScene addComaScene(Engine& en, Real d)
{
    Real qRadius = 2E-1;
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
        // smooth mirror side silver, rough back dull grey
        pars::innerPictureColor = Color(0.85, 0.85, 0.9),
        pars::outerPictureColor = Color(0.35, 0.32, 0.3),
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

    en.addSource(src);
    en.addDevice(qs);
    en.addDevice(scn);
    en.addDevice(floor);

    return { scn, floor };
}

// red cross marking a point, with a gap in the middle so the point stays visible
void drawCross(Bitmap& b, Real x, Real y)
{
    Color c = Color::red();
    b.draw(c, x - 18, x - 4, y - 1, y + 2);
    b.draw(c, x + 5, x + 19, y - 1, y + 2);
    b.draw(c, x - 1, x + 2, y - 18, y - 4);
    b.draw(c, x - 1, x + 2, y + 5, y + 19);
}

// the spot in the focal plane for a 1 mrad field angle
void comaSpot()
{

    Real d = 1E-3;
    Real scn_wind = 1E-4;

    Engine en;
    auto [scn, floor] = addComaScene(en, d);
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

    // the chief ray lands at x = f * d = d / 2, the centre of the window
    char imagePoint[64];
    snprintf(imagePoint, sizeof(imagePoint), "(%g, 0)", d / 2);
    scnbmp.draw(imagePoint,
        TextPaint(pars::color=Color::white(0.2),
            pars::fontSize=15,
            pars::verticalAlign = VerticalAlign::Center,
            pars::horizentalAlign = HorizentalAlign::Right),
        226, 250);

    // cross at the centre of the window (the chief-ray image point) and a
    // frame around the window
    drawCross(scnbmp, 250, 250);
    Color frame = Color::white(0.5);
    scnbmp.draw(frame, 0, 500, 0, 2);
    scnbmp.draw(frame, 0, 500, 498, 500);
    scnbmp.draw(frame, 0, 2, 0, 500);
    scnbmp.draw(frame, 498, 500, 0, 500);

    scnbmp.draw("2E-5",
        TextPaint(pars::color = Color::white(),
            pars::fontSize = 15,
            pars::verticalAlign = VerticalAlign::Center,
            pars::horizentalAlign = HorizentalAlign::Center),
        470, 470);
    // the window is 2 * scn_wind wide over 500 pixels
    Real bar = 2E-5 / (2 * scn_wind) * 500;
    scnbmp.draw(Color::white(), 470 - bar / 2, 470 + bar / 2, 485, 486);

    scnbmp.write("output/coma_spot.png");

    floor->raster("output/coma_floor.png", ScreenOpts(
        pars::gray = true,
        pars::width = 200,
        pars::high = 200,
        pars::n1 = Vec3{ 1,0,0 },
        pars::n2 = Vec3{ 0,1,0 },
        pars::screenSize = 1.
     ));

    en.devicesPicture("output/coma_devices.png", PictureOpts(
        pars::width = 1000,
        pars::high = 1000,
        pars::origin = Vec3{ 3,0, 1 },
        pars::lookAt = Vec3{ 0,0,0 },
        pars::fieldOfView = 1.,
        pars::antiAliasLevel = 2,
        pars::lightOrigin = Vec3{ 0,0,2 }
    ));
}

// side view of the setup with the light source and a fan of traced rays
void comaSetup()
{
    Real d = 1E-3;

    Engine en;
    addComaScene(en, d);

    auto tracker = std::make_shared<Tracking>(0.002);
    tracker->setPictureColor(Color(1, 0.8, 0));
    en.addRecorder(tracker);
    Vec3 dir = normalize(Vec3{ d, 0, -1 });
    for (int i = -4; i <= 4; ++i) {
        Vec3 o{ 0.045 * i, 0, 1 };
        en.emit(Ray(o, dir, 1., 600, randomNorm(dir), 0));
    }
    en.addDevice(tracker);

    // the emitting area of the source, added after the rays are traced so
    // it only shows in the picture
    auto source = planeSurface(
        pars::origin = Vec3{ 0, 0, 1 },
        pars::direction = Vec3{ 0, 0, 1 },
        pars::name = "source");
    source->setBound(boxBound(
        pars::x0 = -0.22,
        pars::x1 = 0.22,
        pars::y0 = -0.22,
        pars::y1 = 0.22));
    source->setPictureColor(Color(1, 0.9, 0.3, 0.6));
    en.addDevice(source);

    Bitmap setup = en.devicesPicture(PictureOpts(
        pars::width = 1000,
        pars::high = 1000,
        pars::origin = Vec3{ 0, -3, 1.3 },
        pars::lookAt = Vec3{ 0, 0, 0.5 },
        pars::fieldOfView = 0.5,
        pars::antiAliasLevel = 2,
        pars::lightOrigin = Vec3{ 1, -2, 2 }
    ));

    TextPaint label(
        pars::color = Color::white(),
        pars::fontSize = 26,
        pars::verticalAlign = VerticalAlign::Center);
    setup.draw("light source, z = 1", label, 680., 176.);
    setup.draw("screen, focal plane z = 0.5", label, 645., 502.);
    setup.draw("parabolic mirror, z = 0", label, 640., 790.);
    setup.write("output/coma_setup.png");
}

// focal-plane spots for several field angles, one row per angle; every row
// shows the same window of the screen, so the spots sit where they land
void comaVsAngle()
{
    Real const angles[] = { 0, 0.5E-3, 1E-3, 2E-3 };
    char const* labels[] = { "0 mrad", "0.5 mrad", "1 mrad", "2 mrad" };
    Real x0 = -1E-4, x1 = 1.2E-3;   // window along x
    Real h = 1.2E-4;                // window height, centred on y = 0
    int rowW = 1300, rowH = 120;    // 1E-6 per pixel
    int labelW = 130, gap = 10, barH = 50;
    Real toPx = rowW / (x1 - x0);

    Bitmap all;
    all.resize(labelW + rowW, 4 * (rowH + gap) + barH);
    all.setBlack();

    TextPaint label(pars::color = Color::white(),
        pars::fontSize = 24,
        pars::verticalAlign = VerticalAlign::Center);

    for (int i = 0; i < 4; ++i) {
        Engine en;
        auto [scn, floor] = addComaScene(en, angles[i]);
        en.emit(10000);

        Bitmap bmp;
        scn->raster(bmp, ScreenOpts(
            pars::gray = true,
            pars::width = rowW,
            pars::high = rowH,
            pars::n1 = Vec3{ 1,0,0 },
            pars::n2 = Vec3{ 0,1,0 },
            pars::n1Min = x0,
            pars::n1Max = x1,
            pars::n2Min = -h / 2,
            pars::n2Max = +h / 2
        ));

        Real top = i * (rowH + gap);
        all.draw(bmp, labelW, labelW + rowW, top, top + rowH);
        // the screen centre (the optical axis) in every row
        drawCross(all, labelW + (0 - x0) * toPx, top + rowH / 2.);
        all.draw(labels[i], label, 10., top + rowH / 2.);
        all.draw(Color::white(0.4), labelW, labelW + rowW, top + rowH, top + rowH + 1);
    }

    // scale bar: 1E-4
    Real barTop = 4 * (rowH + gap) + 20;
    all.draw(Color::white(), labelW, labelW + 1E-4 * toPx, barTop, barTop + 2);
    all.draw("1E-4", label, labelW + 1E-4 * toPx + 10, barTop + 1);

    all.write("output/coma_vs_angle.png");
}

int main(int argc, char* argv[])
{
    init_examples(argc, argv);

    run(comaSpot());
    run(comaSetup());
    run(comaVsAngle());

    return 0;
}
