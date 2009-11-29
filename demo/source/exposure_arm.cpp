#include "exposure.h"
#include "math.h"

//#define SEE_GLOW_VALUES


namespace exposure {
	const u16*  xpal;

	s32   low, high;
	s32   scale;
	s32   glow_low, glow_high;
	s32   glow_scale;


	// no glow
	void  update_noglow(s32 low, s32 high, const u16* xpal, u16* pal_dst) {
		update(low, high, 0, 0, xpal, pal_dst);		
	}

	// glow range is mapped in exposed colorspace
	void  update(s32 low, s32 high, s32 glow_low, s32 glow_high, const u16* xpal, u16* pal_dst, int color_count) {
		exposure::scale = rcp(high-low);
		exposure::low   = low; 
		exposure::high  = high; 

		exposure::glow_scale = rcp(glow_high - glow_low);
		exposure::glow_low   = glow_low;
		exposure::glow_high  = glow_high;
		
		
		// remap glow into range of exposed colors
//		if (glow_low != glow_high) {
//			exposure::glow_scale = rcp(glow_high - glow_low);
//			exposure::glow_low  = MUL16(glow_low, high-low) + low;
//		}
		

		// set globals
		exposure::xpal = xpal;
		
		// quantisize palette
		FORI(color_count) {
			s32 r, g, b;
#ifdef SEE_GLOW_VALUES
			s32 v = getGlowValueFromIndex(i);
			r = g = b = getExposedGlowValue(v) << 4;
#else
			getRGBValuesFromIndex(r, g, b, i);
			exposeRGBValues(r, g, b);
#endif
			*pal_dst++ = getQuantisizedRGBValues(r, g, b);
		}

	}



	void makeXPal(u16 dst[256*4], const u16 src[256]) {
		FORI(256) {
			if (i==0) {
				*dst++ = 0;
				*dst++ = 0;
				*dst++ = 0;
				*dst++ = 0;
				continue;
			}
			int r = (src[i]>>10)&0x1F;
			int g = (src[i]>>5)&0x1F;
			int b = src[i]&0x1F;
			// to 8 bpp
			r = (r<<3)|(r>>2);
			g = (g<<3)|(g>>2);
			b = (b<<3)|(b>>2);
			// to 16 bpp
			r |= r<<8;
			g |= g<<8;
			b |= b<<8;
			
			int v = MUL16(r+g+b, 0x5555);
			*dst++ = v;
			*dst++ = r;
			*dst++ = g;
			*dst++ = b;
		}
	}


};
