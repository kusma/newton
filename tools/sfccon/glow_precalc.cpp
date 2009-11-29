#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "loadsave.h"

#define GLOWGRID_W 31
#define GLOWGRID_H 21

void Blur(std::vector<double>& fb, int width, int height, double blur) 
{
	vector<double> buff;
	buff.resize(max(width, height));
	double val;
	
	// vertical
	for (int x=0; x<width; x++) {		
		// blur downwards
		val = fb[ x + 0*width ];
		buff[0] = val;
		for (int y=1; y<height; y++) {
			const double& v = fb[x+y*width];
			val = v + blur*(val-v);
			buff[y] = val;
		}
		// blur upwards
		val = fb[ x + (height-1)*width ];
		fb[ x + (height-1)*width ] = (val + buff[height-1]) * 0.5;
		for (int y=height-2; y>=0; y--) {
			double& v = fb[x+y*width];
			val = v + blur*(val-v);
			v = (buff[y] + val) * 0.5f;				
		}
	}	

	// horizontal
	for (int y=0; y<height; y++) {		
		// blur to the right
		val = fb[ 0 + y*width ];
		buff[0] = val;

		for (int x=1; x<width; x++) {
			const double& v = fb[x+y*width];
			val = v + blur*(val-v);
			buff[x] = val;
		}

		// blur to the left
		val = fb[ (width-1) + y*width ];
		fb[ (width-1) + y*width ] = (buff[width-1] + val) * 0.5;
		for (int x=width-2; x>=0; x--) {
			double& v = fb[x+y*width];
			val = v + blur*(val-v);
			v = (buff[x] + val) * 0.5f;				
		}
	}
	
}
 
void task_glow_precalc() {
	FIMULTIBITMAP* anim = inputanim;
	float thresh=0.5;
	float gain=2.0;
	float blur=1.5;
	int frameskip=1;
	sscanf(args[1].c_str(), "%f", &thresh);
	sscanf(args[2].c_str(), "%f", &gain);
	sscanf(args[3].c_str(), "%f", &blur);
	sscanf(args[4].c_str(), "%i", &frameskip);
	

	int frameCount = FreeImage_GetPageCount(anim);
	vector<double> fb;
	for (int frame=0; frame<frameCount/frameskip; frame++) {
		FIBITMAP* fiBitmap = FreeImage_LockPage(anim,  frame*frameskip);
		FIBITMAP* grayBitmap = FreeImage_ConvertToGreyscale(fiBitmap);
		FreeImage_UnlockPage(anim, fiBitmap, FALSE);


		int width  = FreeImage_GetWidth(grayBitmap);
		int height = FreeImage_GetHeight(grayBitmap);


		// put into an array of doubles (phew)
		fb.resize(width*height);
		for (int y=0; y<height; y++) {
			BYTE* src = FreeImage_GetScanLine(grayBitmap, y);
			double* dst = &fb[y*width];
			for (int x=0; x<width; x++) {
				*dst++ = *src++ * (1.0/255);
			}
		}

		// threshold
		for (int i=0; i<(int)fb.size(); i++) {
			double& v = fb[i];
			v = (v-thresh)*gain;
			if (v<0) v=0;
		}

		// blur
		Blur(fb, width, height, blur);

		// put it back into greyBitmap
		for (int y=0; y<height; y++) {
			BYTE* dst = FreeImage_GetScanLine(grayBitmap, y);
			double* src = &fb[y*width];
			for (int x=0; x<width; x++) {
				double v = *src++ * 256;
				if (v<0) v=0;
				if (v>255) v=255;
				*dst++ = int(v);
			}
		}

		// downsample it to glowgrid
		FIBITMAP* smallBitmap = FreeImage_Rescale(grayBitmap, GLOWGRID_W, GLOWGRID_H, FILTER_BSPLINE);

		// print result (should use 4bpp not 8?)
		for (int y=0; y<GLOWGRID_H; y++) {
			BYTE* src = FreeImage_GetScanLine(smallBitmap, y);
			for (int x=0; x<GLOWGRID_W; x++) {
				putchar( *src++ );
			}
		}


	}


}
