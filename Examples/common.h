// Shared parts of the example programs: quality levels, helpers used by
// more than one topic, and a small runner that lets each program run a
// subset of its examples chosen on the command line.
#pragma once

#include "../srt/srt.h"
using namespace srt;

#include <memory>
#include <utility>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>
#include <cstdio>
#include <cmath>

int const kFAST = 0;
int const kGOOD = 1;
int const kBEST = 2;

#define nassert(x, y, t) if (fabs(x - y) > t){ printf("test failed:%d\n", __LINE__);}


// width=height=depth=1
inline std::shared_ptr<Surface> boxSurface() {
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

inline void addRoom(Engine &en)
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

// examples picked on the command line; empty means run all
inline std::vector<std::string> gSelected;

inline bool selected(char const* s)
{
    if (gSelected.empty()) {
        return true;
    }
    std::string call = s;
    std::string name = call.substr(0, call.find('('));
    for (auto const& sel : gSelected) {
        if (sel == call || sel == name) {
            return true;
        }
    }
    return false;
}

inline void do_run(void (f)(), char const* s)
{
    if (!selected(s)) {
        return;
    }
    printf("%30s ", s);
    fflush(stdout);
    auto t0 = std::chrono::high_resolution_clock::now();
    f();
    auto t1 = std::chrono::high_resolution_clock::now();
    using seconds = std::chrono::duration<double>;
    double d = seconds(t1 - t0).count();
    printf("%10fs\n", d);
    fflush(stdout);
}

#define run(f) do_run([](){ f; }, #f)

// usage: <program> [name ...]
// name is a function (e.g. blueSky) or a call (e.g. "blueSky(kBEST)");
// with no names every example in the program runs
inline void init_examples(int argc, char* argv[])
{
    gSelected.assign(argv + 1, argv + argc);
    std::filesystem::create_directory("./output");
}

