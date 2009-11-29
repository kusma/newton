#include "main.h"	
#include <assert.h>


struct PixelPrecalc {
	u8 u, v;     
};

void task_tablefx() {
	for (int y=0; y<200; y++) {
		for (int x=0; x<320; x++) {
			float fx = (x-160);    
			float fy = (y-100);    
			float angle = atan2(fx, fy);
			float radius = 2*sqrt(fx*fx+fy*fy);
			angle *= 256.f/(2*M_PI);
			float dist = 256.f*100 / radius;
			PixelPrecalc pp;
			pp.u = (u8)angle;
			pp.v = (u8)dist;
			fwrite(&pp, 2, 1, stdout);
		}    
	}

}