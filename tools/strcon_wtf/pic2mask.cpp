#include "main.h"	
#include "compress.h"



void task_pic2mask() {
	CImage& img = inputimage;


	// konverter
	if (img.GetBPP()!=8) error("Conversion not supported for this pixel format.");
	

	vector<unsigned char> data;

	// dekomprimer bilde
	for (int y=0; y<img.GetHeight(); y++) {
		unsigned char* src = (unsigned char*)img.GetBits() + y*img.GetPitch();
		for (int count=img.GetWidth()*img.GetBPP()/8; count; count--, src++) {
			data.push_back(*src);
		}
	}

	// komprimer det
	vector<unsigned short> komprimerad;
	comp::rleImage(komprimerad, &data[0], &data[0] + data.size(), 4, false);

	// skriv det
	FORI(komprimerad.size()) fwrite(&komprimerad[i], 1, 2, stdout);
}

