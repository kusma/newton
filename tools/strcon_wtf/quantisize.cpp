#include <assert.h>
#include "main.h"
#include "Vector3.h"
#include <set>
#include <list>
#include <algorithm>
#include "freeimage.h"
using namespace std;


#define SCALE_R 0.299
#define SCALE_G 0.587
#define SCALE_B 0.114



FIBITMAP* LoadBitchmap(const char* filename);


static Vector3 colorDistVec(const Vector3& a, const Vector3& b) {
	Vector3 distVec = a-b;
	distVec.x *= SCALE_R;
	distVec.y *= SCALE_G;
	distVec.z *= SCALE_B;
	return distVec;
}

static void quantisize_priv(vector<Vector3> pal, string img_filename, bool dither) {
	FreeImage_Initialise();

	FIBITMAP* dib_orig = LoadBitchmap(img_filename.c_str());
	FIBITMAP* dib = FreeImage_ConvertToRGBF(dib_orig);
	if (!dib) error("Couldn't convert to RGBF: %s", img_filename.c_str());

	int bpp    = FreeImage_GetBPP(dib);
	int width  = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);
	vector<double> color_prob;
	color_prob.resize(pal.size());

	for (int y=0; y<height; y++) {
		float* linep = (float*)FreeImage_GetScanLine(dib, height-y-1);
		for (int x=0; x<width; x++) {
			float* pixel = linep + x*3;
			Vector3 col = Vector3(pixel[0], pixel[1], pixel[2]);

			int selectedColor_Index;
			if (!dither) {
				// find closest color
				double minDist = 10000000000000000000;
				FORI(pal.size()) {
					if (i==0) continue; // cannot use color 0 :-/
					double dist = colorDistVec(pal[i], col).magnitude_pow();
					if (dist<minDist) {
						minDist = dist;
						selectedColor_Index = i;
					}
				}
			} else {
				// find probability for each color
				double total_prob = 0.0;
				FORI(pal.size()) {
					double prob = colorDistVec(pal[i], col).magnitude();
					if (prob < 0) prob = 0;
					total_prob += prob;
					color_prob[i] = prob;
				}

				// fetch a random number to select color
				double rand_selector = (rand()&0xFFFF) * (1.0/0xFFFF) * total_prob;
				
				// loop through colors to find one
				double total_sum = 0.0;
				FORI(pal.size()) {
					total_sum += color_prob[i];
					if (total_sum >= rand_selector) {
						selectedColor_Index = i;
						break;
					}
				}

			}

			fwrite(&selectedColor_Index, 1, 1, stdout);
		}
	}
	FreeImage_DeInitialise();
}


void task_quantisize() {
	quantisize_priv(palette_float, args[1], args[2]=="1");
}

/*
void task_quantisize_xpal() {
	vector<Vector3> fpal;

	string& pal_filename = args[0];
	fprintf(stderr, "Loading HDR XPal palette..\n");
	FILE* fil = fopen(pal_filename.c_str(), "rb");
	if (!fil) error("File not found: %s\n\n", pal_filename.c_str());

	// read size
	int count=0;
	fread(&count, 2, 1, fil);
	fpal.resize(count);
	FORI(count) {
		Vector3& fcol = fpal[i];

		unsigned short hei[4];
		size_t ret = fread(hei, 2, 4, fil);
		if (!ret) error("File is too short!");

		fcol.x = hei[1] * (1.0/0xFFFF);
		fcol.y = hei[2] * (1.0/0xFFFF);
		fcol.z = hei[3] * (1.0/0xFFFF);
	}
	fclose(fil);

	quantisize_priv(fpal, args[1], args[2]=="1");
}
*/
