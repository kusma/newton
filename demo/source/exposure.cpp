#include "exposure.h"
#include "math.h"

namespace exp {
	const u16*  xpal;
	u32   exposure;
	u32   threshold;
	u32   glowscale;


	void  update(u32 threshold, u32 glowstrength, const u16* xpal, u16* pal_dst) {
		// set globals
		exp::exposure  = rcp(threshold);
		exp::threshold = threshold;
		exp::glowscale = MUL16(rcp(0xFFFF-threshold), glowstrength);
		exp::xpal = xpal;

		// quantisize palette
		FORI(256) {
			u16 r, g, b;
			getRGBValuesFromIndex(r, g, b, i);
			exposeRGBValues(r, g, b);
			*pal_dst++ = getQuantisizedRGBValues(r, g, b);
		}
	}


	void getRGBValuesFromIndex(u16& r, u16& g, u16& b, int index) {
		assert(index>=0 && index<256);
		r = xpal[index+1];
		g = xpal[index+2];
		b = xpal[index+3];
	}
	
	u16   getGlowValueFromIndex(int index) {
		assert(index>=0 && index<256);
		return xpal[index];
	}

	void  exposeRGBValues(u16& r, u16& g, u16& b) {
		r = MUL16(r, exposure);
		g = MUL16(g, exposure);
		b = MUL16(b, exposure);
	}

	u16  getExposedGlowValue(u32 v) {
		// suptract threshold and clamp bottom
		v = v - threshold;
		if (v<0) return 0;
		// divide by remaining range and scale by glowstrength
		v = MUL16(v, glowscale);
		// clamp top and return
		if (v>>16) return 0xFFFF;
		return v;		
	}

	u16  getQuantisizedRGBValues(u32 r, u32 g, u32 b) {
		// clamp
		if (r>0xFFFF) r=0xFFFF;
		if (g>0xFFFF) g=0xFFFF;
		if (b>0xFFFF) b=0xFFFF;
		// skal ha 5 bit - skift ned 11 bits
		r >>= 11;
		g >>= 11;
		b >>= 11;
		return RGB5(r,g,b);
	}

	u8    getQuantisizedGlowValue(u32 v) {
		// clamp
		if (v>0xFFFF) v=0xFFFF;
		// skal ha 3 bit - skift ned 13 bits
		int noiz = cheap_rand()&((1<<12)-1);
		v = (v + noiz)>>13;
		return v;
	}

};
