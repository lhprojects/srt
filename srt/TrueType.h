#pragma once
#include "stb_truetype.h"
#include "Real.h"
#include "Vec3.h"
#include <string>

namespace srt
{
	// to use TrueType, you need to implement the Blit
	struct Blit
	{
		// blit a rect (data,w,h,stride) to position x0, y0
		// for i in w
		//     for j in h
		//         bitmap[x0+i,y0+j] <- data[i*stride+j]
		virtual void blit(int x0, int y0, void* data,
			int w, int h, int stride) = 0;
	};

	struct TrueType
	{
	private:
		unsigned char* fontBuffer = nullptr;
		stbtt_fontinfo info;

	public:
		TrueType(std::string const& filename);
		~TrueType();

		// load TrueType from file
		int load(std::string const& filename);

		// length of a string
		// pixels: the hight of a world
		double length(char const* word,
			Real pixels);

		void draw(
			char const* word,
			Real pixels,
			int x0,
			int y0,
			int x1,
			Blit& bilt);
	};

}
