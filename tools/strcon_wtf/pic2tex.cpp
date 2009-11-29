#include "main.h"	


void task_pic2tex() {
	CImage& img = inputimage;

	// advar eigil
	if (img.GetBPP()!=8 || img.GetWidth()!=256 || img.GetHeight()!=256) fprintf(stderr, "Warning: Texture should be 256x256 8bpp\n");


	// konverter
	if (img.GetBPP()==8) {
		for (int y=0; y<img.GetHeight(); y++) {
			unsigned char* src = (unsigned char*)img.GetBits() + y*img.GetPitch();
			for (int count=img.GetWidth()*img.GetBPP()/8; count; count--, src++) putchar(*src);
		}
	} else {
		error("Conversion not implemented for this pixel format.");
	}
}




void task_pic2symraw() {
	CImage& img = inputimage;

	// advar eigil
	if (img.GetWidth()!=256 || img.GetHeight()!=256) fprintf(stderr, "Warning: Normal map should be 256x256\n");


	// konverter - should be replaced by GetPixel(x, y)
	if (img.GetBPP()==24 || img.GetBPP()==32) {
		for (int y=0; y<img.GetHeight(); y++) {
			unsigned char* src = (unsigned char*)img.GetBits() + y*img.GetPitch();
			for (int x=0; x<img.GetWidth(); x++) {
				int r = *src++;
				int g = *src++;
				int b = *src++;
				if (img.GetBPP()==32) src++;

				r>>=3;
				g>>=2;
				b>>=3;

				int res = (r<<11)|(g<<5)|(b);
				fwrite(&res, 1, 2, stdout);	
			}
		}
	} else {
		error("Conversion not implemented for this pixel format.");
	}
}

