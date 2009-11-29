#include "main.h"	

#define RGB5(r,g,b)	((r)|((g)<<5)|((b)<<10))
//#define RGB8(r,g,b)	( (((b)>>3)<<10) | (((g)>>3)<<5) | ((r)>>3) )


void task_actpal() {
//	CImage& img = inputimage;
//	if (img.GetBPP()!=8 || img.GetMaxColorTableEntries()<=0) error("Image must be indexed 8bpp to extract palette");
	saveAct(palette);
}

void task_amigapal() {
	FORI(palette.size()) {
		Color8& c = palette[i];
		int tom=0;
		fwrite(&tom, 1, 1, stdout);
		fwrite(&c.rgb[0], 1, 1, stdout);
		fwrite(&c.rgb[1], 1, 1, stdout);
		fwrite(&c.rgb[2], 1, 1, stdout);
	}
}

void task_gbapal() {
	FORI(palette.size()) {
		Color8& c = palette[i];
		int r = c.rgb[0]>>3;
		int g = c.rgb[1]>>3;
		int b = c.rgb[2]>>3;
		unsigned short rgb = RGB5(r, g, b);
		fwrite(&rgb, 1, 2, stdout);
	}
}


void task_gbaxpal() {
	int count = palette.size();
	fwrite(&count, 1, 2, stdout);
	FORI(palette.size()) {
		Color8& c = palette[i];
		int r = c.rgb[0];
		int g = c.rgb[1];
		int b = c.rgb[2];
		r = (r<<8) | r;
		g = (g<<8) | g;
		b = (b<<8) | b;
		int x = (r+g+b)/3;
		
		fwrite(&x, 1, 2, stdout);
		fwrite(&r, 1, 2, stdout);
		fwrite(&g, 1, 2, stdout);
		fwrite(&b, 1, 2, stdout);
	}
}

/*
void task_fadepal() {
	FORI(palette.size()) {
		Color8& c = palette[i];
		int r = c.rgb[0]>>3;
		int g = c.rgb[1]>>3;
		int b = c.rgb[2]>>3;
		const int thresh = 12;
#if 0
		for (int scale=0; scale<thresh; scale++) {
			float fscale = scale * (1.f/thresh);
			unsigned short rgb = RGB5(int(r*fscale), int(g*fscale), int(b*fscale));
			fwrite(&rgb, 1, 2, stdout);
		}	
#else
		for (int scale=0; scale<thresh; scale++) {
			float fscale = scale * (0.80f/thresh) + 0.20;
			unsigned short rgb = RGB5(int(r*fscale), int(g*fscale), int(b*fscale));
			fwrite(&rgb, 1, 2, stdout);
		}	
#endif
		for (int scale=thresh; scale<16; scale++) {
			float fscale = float(scale-thresh + 1) / (16-thresh);

			int r_ = r*(1-fscale) + (0x1F*fscale);
			int g_ = g*(1-fscale) + (0x1F*fscale);
			int b_ = b*(1-fscale) + (0x1F*fscale);
			unsigned short rgb = RGB5(r_, g_, b_);
			fwrite(&rgb, 1, 2, stdout);
		}	
	}
}

*/