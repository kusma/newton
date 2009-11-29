#pragma once
#include "support.h"
#include "assert.h"
#include "math.h"

namespace exposure {
	// this should really be private,
	// but then i can't inline functions :-P
	extern const u16*  xpal;
	extern s32   low;
	extern s32   scale;
	extern s32   high;
	extern s32   glow_low;
	extern s32   glow_scale;
	extern s32   glow_high;


	
	
	INLINE void  getRGBValuesFromIndex(s32& r, s32& g, s32& b, int index) {
		assert(index>=0 && index<256);
		r = xpal[index*4+1];
		g = xpal[index*4+2];
		b = xpal[index*4+3];
	}

	INLINE s32   getGlowValueFromIndex(int index) {
		assert(index>=0 && index<256);
		return xpal[index*4];
	}

	INLINE void  exposeRGBValues(s32& r, s32& g, s32& b) {
		r -= low; 
		g -= low; 
		b -= low; 
		if (r<0) r=0; else r = MUL16(r, scale);
		if (g<0) g=0; else g = MUL16(g, scale);
		if (b<0) b=0; else b = MUL16(b, scale);
	}
	
	// returns an unsigned 4:12 number
	INLINE s32   getExposedGlowValue(s32 v) {
		// suptract threshold and clamp bottom
		v = v - glow_low;
		if (v<0) return 0;


		// divide by remaining range and scale by glowstrength
		// turn from 0:16 to 4:12
		v = MUL16(v, glow_scale)>>4;
//		if (v>0xFFF) v=0xFFF; // WTF?? I threw away 4 bits??
		if (v>0xFFFF) v=0xFFFF;
		return v;
	}

	INLINE u16   getQuantisizedRGBValues(s32 r, s32 g, s32 b) {
		// clamp
		if (r>0xFFFF) r=0xFFFF;
		if (g>0xFFFF) g=0xFFFF;
		if (b>0xFFFF) b=0xFFFF;
		// skal ha 5 bit - skift ned 11 bits
		r >>= 8;
		g >>= 8;
		b >>= 8;
		return RGB8(r,g,b);
	}
	
	INLINE u8    getQuantisizedGlowValue(s32 v, int sign) {
		// har 4:12.
		// skal ha 3 bit - skift ned 9 bits og clamp
		
		// dither
#if 0
		int noiz = MUL16(cheap_rand() & 0xFFFF, 0xB00);
		if (sign) noiz += 0x1000;
		v += noiz;
#else
		if (sign) v += 0x100;
#endif
		
		// clamp
		if (v>0xFFF) v=0xFFF;

		// scale down
		v >>= 9;
		return v;
	}

	// glow range is mapped in exposed colorspace
	void  update(s32 low, s32 high, s32 glow_low, s32 glow_high, const u16* xpal, u16* pal_dst=BG_COLORS, int color_count=256);
	void  update_noglow(s32 low, s32 high, const u16* xpal, u16* pal_dst=BG_COLORS);
	
	// utility functions
	void makeXPal(u16 dst[256*4], const u16 src[256]); 
};
