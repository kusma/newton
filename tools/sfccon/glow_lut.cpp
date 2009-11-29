#include "main.h"

#define RGB5(r,g,b)	((r)|((g)<<5)|((b)<<10))
#define RGB8(r,g,b)	( (((b)>>3)<<10) | (((g)>>3)<<5) | ((r)>>3) )

#undef max
#undef min
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))


typedef	unsigned char			u8;
typedef	unsigned short int		u16;
typedef	unsigned int			u32;
typedef	signed char				s8;
typedef	signed short int		s16;
typedef	signed int				s32;


namespace {

	void bildedings(const char* filnavn, int size, int* liste, int count) {
		FILE* fil = fopen(filnavn, "wb");
		if (!fil) error("Could not create \"%s\"", filnavn);
		for (int i=0; i<count; i++) {
			int val = liste[i];
			fwrite(&val, 1, size, fil);
		}
		fclose(fil);
	}

	// interpolate sprite recursively from 4 corners using MEDIAN
	void recurse_gradient(int* p, int W, int H, int pitch, int v00, int v02, int v20, int v22, int s_spr) {
		int v01 = (v00+v02)/2;
		int v10 = (v00+v20)/2;
		int v21 = (v20+v22)/2;
		int v12 = (v02+v22)/2;
		int s0=v00;
		int s1=v02;
		int s2=v20;
		int s3=v22;
		FORI(3) {
			if (s2>s3) std::swap(s2, s3);
			if (s1>s2) std::swap(s1, s2);
			if (s0>s1) std::swap(s0, s1);
		}
		int v11 = (s1+s2)/2;
		if (W==0 && H==0) {
			int noizeval = (rand()%32)-15;
			int v = v00+noizeval;
			if (v<0) v=0;
			if (v>255) v=255;
			*p = v;
			return;
		}
		int w = W/2;
		int h = H/2;
		int o = w*pitch;
		recurse_gradient(p,     w, h, pitch, v00, v01, v10, v11, s_spr);
		recurse_gradient(p+w,   w, h, pitch, v01, v02, v11, v12, s_spr);
		recurse_gradient(p+o,   w, h, pitch, v10, v11, v20, v21, s_spr);
		recurse_gradient(p+o+w, w, h, pitch, v11, v12, v21, v22, s_spr);
	}





	// get pretty gradient
	int palval(int v) {
		static const u16 glowgrad[0x20] = {
			0x0000, 0x0000, 0x0421, 0x0842, 0x0c63, 0x1084, 0x14a5, 0x18c5, 
			0x1ce6, 0x2105,	0x2525, 0x2546, 0x2966, 0x2d66, 0x3186, 0x35a7, 
			0x39c7, 0x3de8, 0x4208, 0x4629,	0x4a4a, 0x4e6b, 0x528c, 0x56ad,
			0x5eef, 0x6311, 0x6b53, 0x7395, 0x77b7, 0x7bd9,	0x7ffa, 0x7ffb,
		};
		if (v>0x1F) v=0x1F;
		if (v<0) v=0;
		return glowgrad[v];
	}


	// This generates the integer-fitted palettes for the 3-bit-per-node glow. 
	// Each palette has 15 colors - index 0 is reserved as transparent :-P
	void prepare_glowpal(int* pal) {

		// hand-coded weights for the first few palettes
		static const double unpal[][15] = {
			{ 0.00, 0.50, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00, 5.50, 6.00, 6.50, 7.00 },
			{ 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00, 5.50, 6.00, 6.50, 7.00, 7.00, 7.00 },
			{ 2.00, 2.50, 3.00, 3.25, 3.50, 3.75, 4.00, 4.50, 5.00, 5.25, 5.50, 5.75, 6.00, 6.50, 7.00 },
			{ 3.00, 3.25, 3.50, 3.75, 4.00, 4.50, 5.00, 5.50, 6.00, 6.25, 6.50, 6.75, 7.00, 7.00, 7.00 },
			{ 4.00, 4.25, 4.50, 4.75, 5.00, 5.17, 5.33, 5.50, 5.67, 5.83, 6.00, 6.25, 6.50, 6.75, 7.00 },
			{ 5.00, 5.17, 5.33, 5.50, 5.66, 5.83, 6.00, 6.17, 6.33, 6.50, 6.66, 6.83, 7.00, 7.00, 7.00 },
			//  6             7
			//  7             7
		};

		// create the integer fitted color table
		{
			int* p=pal;
			// the first 6 are hand-coded in the unpal table:
			FORI(6) {
				*p++ = 0;
				FORJ(15) {
					int v = int(unpal[i][j]*256/7.00);
					v = (v+2)>>3;
					*p++ = v;
				}
			}
			// lerp the rest (integer distances, no fit req)
			for (int i=6; i<8; i++) {
				*p++ = 0;
				FORJ(15) {
					int v = int(((7-i)*(j/14.0) + i) * 256.0/7.0);
					v = (v+2)>>3;
					*p++ = v;
				}
			}
		}

		// make 8 inverted palettes
		FORI(8) {
			int* src = &pal[i*16];
			int* dst = src+8*16;
			FORJ(16) {
				*dst++ = 0x1F - *src++;
			}
		}

		// clamp
		FORI(256) {
			int& v = pal[i];
			if (v>0x1F) v=0x1F;
			if (v<0) v=0;
		}

		// apply pretty color gradient from palval 
		// (should be moved to GBA for flexibility)
	//	FORI(256) pal[i] = palval(pal[i]);

	}


	// creates the smooth 3-bit-per-node glowsprites
	void prepare_glowsprites(int* p) {
		FORI(512) {
			// første vekt er alltid 0.
			// finn de 3 resterende vektene fra index.
			int i1 = ((i)&0x7);
			int i2 = ((i>>3)&0x7);
			int i3 = ((i>>6)&0x7);

			int v0 = 0;
			int	v1 = (i1<<5)|(i1<<2)|(i1>>1);
			int	v2 = (i2<<5)|(i2<<2)|(i2>>1);
			int	v3 = (i3<<5)|(i3<<2)|(i3>>1);
			v0 = (v0*0xEF/0xFF)+0x10;
			v1 = (v1*0xEF/0xFF)+0x10;
			v2 = (v2*0xEF/0xFF)+0x10;
			v3 = (v3*0xEF/0xFF)+0x10;

			// skriv
			recurse_gradient(p, 8, 8, 8, v0, v1, v2, v3,i);
			p+=8*8;
		}
	}

#if 0
	// creates shitty 2-bit-per-node glowsprites
	void prepare_glowsprites_fast(int* p) {
		FORI(256) {
			// første vekt er alltid 0.
			// finn de 3 resterende vektene fra index.
			int v0 = ((i)&0x3);
			int v1 = ((i>>2)&0x3);
			int v2 = ((i>>4)&0x3);
			int v3 = ((i>>6)&0x3);
			v0 = (v0<<6)|(v0<<4)|(v0<<2)|v0;
			v1 = (v1<<6)|(v1<<4)|(v1<<2)|v1;
			v2 = (v2<<6)|(v2<<4)|(v2<<2)|v2;
			v3 = (v3<<6)|(v3<<4)|(v3<<2)|v3;
			// skriv
			recurse_gradient(p, 8, 8, 8, v0, v1, v2, v3,i);
			p+=8*8;
		}
	}
#endif








	int glowpal[256];
	int glowsprites[512*8*8];
	int glowsprites_fast[512*8*8];
	int glowsprites_conc[512*8*8/2];

	void concat(int* dst, int* src, int count) {
		FORI(count/2) {
			int v0 = (src[i*2])>>4;
			int v1 = (src[i*2+1])>>4;
			if (v0<1) v0=1;
			if (v1<1) v1=1;
			if (v0>15) v0=15;
			if (v1>15) v1=15;
			dst[i] = (v1<<4)|v0;
		}
	}


	void lag_luts(const char* glowpalf, const char* glowspritesf) {
		prepare_glowpal(glowpal);
		prepare_glowsprites(glowsprites);

		// write glow stuff
		bildedings(glowpalf, 1, glowpal, 256);
		concat(glowsprites_conc, glowsprites, 512*8*8);
		bildedings(glowspritesf, 1, glowsprites_conc, 512*8*8/2);

		// write debug data
#if 1
		for (int i=0; i<512*8*8; i++) {
			glowsprites[i]-=0x10;
			glowsprites[i]>>=4;
			glowsprites[i]<<=4;
			glowsprites[i]+=0x10;
		}
		bildedings("c:/glowsprites_debug.raw", 1, glowsprites, 512*8*8);
#endif
	}

};

void task_glow_lut() {
	string dir = args[0];
	if (dir.size() && dir.find_last_of("/\\")!=dir.size()-1) dir += "/";
	// do it
	string glowpal = dir + "glow.pali";
	string glowsprites = dir + "glow.spr";
	lag_luts(glowpal.c_str(), glowsprites.c_str());
	printf("success\n");
}