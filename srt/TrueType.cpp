#define _CRT_SECURE_NO_WARNINGS
#include "TrueType.h"
#include <vector>

namespace srt {


	TrueType::TrueType(std::string const& filename)
	{
		load(filename);
	}

	TrueType::~TrueType()
	{
		if (fontBuffer) {
			free(fontBuffer);
		}
	}

	// load TrueType from file
	int TrueType::load(std::string const& filename)
	{
		FILE* fontFile = fopen(filename.c_str(), "rb");
		if (fontFile == NULL) {
			throw "not exist";
		}
		fseek(fontFile, 0, SEEK_END);
		size_t size = ftell(fontFile);
		fseek(fontFile, 0, SEEK_SET);

		fontBuffer = (unsigned char*)calloc(size, sizeof(unsigned char));
		if (!fontBuffer) {
			throw "not exist";
		}
		fread(fontBuffer, size, 1, fontFile);
		fclose(fontFile);
		if (!stbtt_InitFont(&info, fontBuffer, 0)) {
			throw "not exist";
		}

		return 0;
	}

	// length of a string
	double TrueType::length(char const* word,
		Real pixels)
	{
		double x0 = 0;
		float scale = stbtt_ScaleForPixelHeight(&info, (float)pixels);

		for (int i = 0; i < strlen(word); ++i) {

			int advanceWidth = 0;
			int leftSideBearing = 0;
			stbtt_GetCodepointHMetrics(&info, word[i],
				&advanceWidth, &leftSideBearing);

			x0 += roundf(advanceWidth * scale);

			int kern;
			kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
			x0 += roundf(kern * scale);
		}
		return x0;
	}

	void TrueType::draw(
		char const* word,
		Real pixels,
		int x0,
		int y0,
		int x1,
		Blit& bilt)
	{
		int ascent = 0;
		int descent = 0;
		int lineGap = 0;

		stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
		float scale = stbtt_ScaleForPixelHeight(&info, (float)pixels);
		ascent = (int)roundf(ascent * scale);
		descent = (int)roundf(descent * scale);

		for (int i = 0; i < strlen(word); ++i) {

			int advanceWidth = 0;
			int leftSideBearing = 0;
			stbtt_GetCodepointHMetrics(&info, word[i],
				&advanceWidth, &leftSideBearing);

			int c_x1, c_y1, c_x2, c_y2;
			stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale,
				&c_x1, &c_y1, &c_x2, &c_y2);

			int bitmap_w = c_x2 - c_x1;
			int bitmap_h = c_y2 - c_y1;
			std::vector<unsigned char> bitmap(size_t(bitmap_w) * bitmap_h);

			stbtt_MakeCodepointBitmap(&info, bitmap.data(),
				c_x2 - c_x1, c_y2 - c_y1, bitmap_w,
				scale, scale,
				word[i]);

			bilt.blit(x0 + c_x1, y0 + ascent + c_y1,
				bitmap.data(), bitmap_w, bitmap_h, bitmap_w);

			x0 += (int)roundf(advanceWidth * scale);

			int kern;
			kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
			x0 += (int)roundf(kern * scale);

			if (x0 > x1) {
				break;
			}
		}
	}
}
