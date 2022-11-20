#include "Engine.h"
#include <stdio.h>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

namespace srt {

	bool Obj::loadOBJ(const char* filepath)
	{
		std::ifstream ifs(filepath, std::ifstream::in);
		if (ifs.eof()) {
			return false;
		}

		while (true) {

			std::string line;
			std::getline(ifs, line);
			if (ifs.eof()) break;
			std::stringstream ss(line);

			std::string key;
			ss >> key;

			if (key == "v") {
				Vec3 Vertex;
				ss >> Vertex.fX >> Vertex.fY >> Vertex.fZ;
				Vtxs.push_back(Vertex);
			} else if (key == "vt") {
				Vec2 UV;
				Real z;
				ss >> UV.fX >> UV.fY;
				ss >> std::ws;
				if (!ss.eof()) ss >> z;
				VtxTextures.push_back(UV);
			}

			else if (key == "vn") {
				Vec3 Normal;
				ss >> Normal.fX >> Normal.fY >> Normal.fZ;
				VtxNormals.push_back(Normal);
			} else if (key == "f") {

				Face f;
				for (;;) {
					FaceVertex fv;
					ss >> fv.VtxIdx >> std::ws;
					fv.VtxIdx -= 1;
					if (ss.peek() == '/') {
						char c;
						ss >> c;
						ss >> fv.VtxTextureIdx;
						fv.VtxTextureIdx -= 1;
						ss >> std::ws;
						if (ss.peek() == '/') {
							ss >> c;
							ss >> fv.VtxNormalIdx;
							fv.VtxNormalIdx -= 1;
						}
					}
					if (ss.eof()) {
						break;
					}
					f.Vtxs.push_back(fv);
				}
				Faces.push_back(f);
			} else if (key == "#") {
				// comment
			} else {
				if (key == "o" || key == "s"
					|| key == "mtllib" || key == "usemtl") {

				} else {
					// error!
					printf("unkown keyword in objfile\n");
				}
			}

		}

		return true;
	}
}
