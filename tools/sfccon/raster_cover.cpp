#include "main.h"

#define SLOPE_PREC 4
#define SLOPE_MAX ((+1<<SLOPE_PREC)-1)
#define SLOPE_MIN ((-1<<SLOPE_PREC))
#define SLOPE_TOFLOAT (1.0 / (1<<SLOPE_PREC))

#define DISTANCE_PREC 2
#define DISTANCE_MAX ((+8<<DISTANCE_PREC)-1)
#define DISTANCE_MIN ((-8<<DISTANCE_PREC))
#define DISTANCE_TOFLOAT (1.0 / (1<<DISTANCE_PREC))

u64 rasterize(vector<u8>& preview, bool horiz, double slope, double distance) {
	int index=0;

	u64 ret = 0;

	FORXY(8, 8) {
		double fx = double(x) - 3.5;
		double fy = double(y) - 3.5;

		bool val;
		if (horiz) {
			val = fy > fx*slope + distance;
		} else {
			val = fx > fy*slope + distance;
		}

		// write
		preview.push_back( val ? 255 : 0);
		if (val) ret |= 1 << index;

		// linear
		index++;
	}


	return ret;
}



void task_raster() {
	vector<u64> dst;
	vector<u8> preview;
	for (int slope=SLOPE_MIN; slope<=SLOPE_MAX; slope++) {
		float fslope = slope * SLOPE_TOFLOAT;
		for (int distance=DISTANCE_MIN; distance<=DISTANCE_MAX; distance++) {
			double fdistance = distance * DISTANCE_TOFLOAT;
			dst.push_back( rasterize(preview, false, -fslope, fdistance) );
		}
	}
	for (int slope=SLOPE_MIN; slope<=SLOPE_MAX; slope++) {
		float fslope = slope * SLOPE_TOFLOAT;
		for (int distance=DISTANCE_MIN; distance<=DISTANCE_MAX; distance++) {
			double fdistance = distance * DISTANCE_TOFLOAT;
			dst.push_back( rasterize(preview, true, fslope, fdistance) );
		}
	}

	FILE* fil = fopen("raster_cover.raw", "wb");
	fwrite(&dst[0], sizeof(u64), dst.size(), fil);
	fclose(fil);
	fil = fopen("raster_cover_preview.raw", "wb");
	fwrite(&preview[0], sizeof(u8), preview.size(), fil);
	fclose(fil);

}