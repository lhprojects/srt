// wavelength, temperature and black-body colors
#include "common.h"

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

int main(int argc, char* argv[])
{
    init_examples(argc, argv);

    run(testPlankLaw());
    run(testT2Color());
    run(testWaveLengthToColor());

    return 0;
}
