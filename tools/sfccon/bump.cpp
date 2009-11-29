#include "main.h"	
#include <assert.h>

#define RGB8(r,g,b)	( (((b)>>3)<<10) | (((g)>>3)<<5) | ((r)>>3) )

namespace {
	enum Mode {
		MODE_NMAP,
		MODE_EBUMP
	};
	void convert(Mode mode) {
		CImage& img = inputimage;

		int bpp = img.GetBPP();
		int w = img.GetWidth();
		int h = img.GetHeight();

		// advar eigil
		if (img.GetWidth()!=256 || img.GetHeight()!=256) fprintf(stderr, "Warning: Normal map should be 256x256\n");


		// konverter - should be replaced by GetPixel(x, y)
		if (img.GetBPP()==24 || img.GetBPP()==32) {
			for (int y=0; y<img.GetHeight(); y++) {
				unsigned char* src = (unsigned char*)img.GetBits() + y*img.GetPitch();
				for (int x=0; x<img.GetWidth(); x++) {
					int vx = (*src++)-128;
					int vy = (*src++)-128;
					int vz = (*src++)-128;
					if (img.GetBPP()==32) src++;

					// swappings;
				//	int vy = -vx_;
				//	int vz =  vy_;
				//	int vx = -vz_;

					if (mode==MODE_NMAP) {
						vx &= 0xFF;
						vy &= 0xFF;
						vz &= 0xFF;
						int res = RGB8(vx, vy, vz)<<1;
						fwrite(&res, 1, 2, stdout);
					} else if (mode==MODE_EBUMP) {
						double z = vx*(2.0/256);
						double y = vy*(2.0/256);
						double x = vz*(2.0/256);
						double s = sqrt(x*x+y*y+z*z);
						if (s<0.01) {
							putchar(128);
							putchar(128);
							continue;
						}
						double s_inv = 1.0/s;
						x*=s_inv;
						y*=s_inv;
						z*=s_inv;

						double h = -asin(z) * (1.0/M_PI) + 0.5;
						double a =  atan2(x, y) * (0.5/M_PI) + 0.5;
						
						assert(h>=-0.2 && h<=1.2);
						assert(a>=-0.2 && a<=1.2);
						int ih = int(h*255);
						int ia = int(a*255);
						if (ih>255) ih=255; 
						if (ih<0) ih=0;
						if (ia>255) ia=255; 
						if (ia<0) ia=0;
						int res = (ia<<8)|ih;
						fwrite(&res, 1, 2, stdout);
					}
				}
			}
		} else {
			error("Conversion not implemented for this pixel format.");
		}
	}
};


void task_pic2nmap() {
	convert(MODE_NMAP);
}
void task_pic2ebump() {
	convert(MODE_EBUMP);
}
