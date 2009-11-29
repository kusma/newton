#include <gba_dma.h>
#include <mappy.h>
#include "poly.h"
#include "support.h"
#include "math.h"
#include "exstructs.h"
#include <gba_systemcalls.h>
#include "fixed.h"



namespace {
	int blend75(int a, int b, const u8* shadeTable) {
		return shadeTable[(a<<8)|shadeTable[(b<<8)|a]];
//		return (a*3+b)>>2;
	}
	int blend50(int a, int b, const u8* shadeTable) {
		return shadeTable[(a<<8)|b];
//		return (a+b)>>1;
	}

	INLINE int blend(int covg, int fg, int bg, const u8* shadeTable) 
	{
		int tmp;
		covg = (covg) >> 13; // 0-7
		switch (covg) {
			case 0: 
				return bg;
			case 1:
			case 2: 
				return blend75(bg, fg, shadeTable);
			case 3:
			case 4: 
				return blend50(bg, fg, shadeTable);
			case 5:
			case 6: 
				return blend75(fg, bg, shadeTable);
			default: 
				return fg;
		}
		/*
		int covg_inv = 0x10000-covg;
		fg *= covg;
		bg *= covg_inv;
		return (fg+bg)>>16;
		*/
	}

	void swap(int& a, int& b) {
		int tmp=a;
		a=b;
		b=tmp;
	}
/*	struct SlopeIter {
		s32 v;
		s32 delta;
		INLINE void increment() {
			v+=delta;
		}
		INLINE void increment(int count) {
			v+=(s64(delta)*count)>>8;
		}
		INLINE void roundup(s32 y, s32 dst_y) // 24:8
		{
			int ceil = (dst_y>>8)<<8;
			int steps = ceil-y;
			v += (s64(delta) * steps)>>8;
		}
		INLINE void roundup16(s32 y, s32 dst_y) // 24:8
		{
			int ceil = (dst_y>>16)<<16;
			int steps = ceil-y;
			v += (s64(delta) * steps)>>16;
		}
	};
*/

	struct Iter {
		s32 v;
		s32 delta;
		INLINE void increment() {
			v+=delta;
		}
		INLINE void increment(int count) {
			v+=(s64(delta)*count)>>8;
		}
	};
	
	int read_pixel(u8* p) {
		u16& v = *(u16*)(int(p)&(~1));
		if (int(p)&1) {
			return (v>>8)&0xFF;
		} else {
			return (v)&0xFF;
		}
	}

	void write_pixel(u8* p, int c) {
//		c=3;
		u16& v = *(u16*)(int(p)&(~1));
		if (int(p)&1) {
			v = (v&0x00FF) | (c<<8);
		} else {
			v = (v&0xFF00) | c;
		}
	}
}


void antialias_vert(s32 x, s32 y, u8* fb, const u8* shadeTable)
{
	const int clip_top=0;
	const int clip_bottom=160;

	int dst_x = (x+0x7F)>>8;
	int dst_y = (y+0x7F)>>8;
	u8* dst = &fb[dst_y*GBA_WIDTH+dst_x];

	if ( dst_x>0 && dst_x<GBA_WIDTH-1 && dst_y>0 && dst_y<GBA_HEIGHT-1) {
		int m = read_pixel( dst );
		int l = read_pixel( dst-1 );
		int r = read_pixel( dst+1 );
		int t = read_pixel( dst-GBA_WIDTH );
		int b = read_pixel( dst+GBA_WIDTH );
		int cross = blend50( blend50(l, t, shadeTable), blend50(b, r, shadeTable), shadeTable );
		int val = blend75(cross, m, shadeTable);
		write_pixel(dst, val);
	} else {
		if (dst_x<0 || dst_x>=GBA_WIDTH) return;
		if (dst_y<clip_top || dst_y>=clip_bottom) return;

		int m = read_pixel( dst );
		int l = read_pixel( dst-1 );
		int r = read_pixel( dst+1 );
		int t = read_pixel( dst-GBA_WIDTH );
		int b = read_pixel( dst+GBA_WIDTH );
		int hSample = blend50(l, r, shadeTable);
		int vSample = blend50(t, b, shadeTable);

		if (dst_x==0) hSample = r;
		else if (dst_x==GBA_WIDTH-1) hSample = l;
		if (dst_y==clip_top) vSample = b;
		else if (dst_y==clip_bottom-1) vSample = t;

		int cross = blend50(hSample,vSample, shadeTable);
		int val = blend75(cross, m, shadeTable);
		write_pixel(dst, val);
	}

}


//void antialias_vert(s32 x, s32 y, s32* edgeVerts, int edgeCount)
/*void antialias_vert(s32 x, s32 y, s32 x0, s32 y0, s32 x1, s32 y1, u8* fb)
{
	int dst_x = (x+0x7F)>>8;
	int dst_y = (y+0x7F)>>8;

	int y0_rcp = rcp((y0-y)<<8); // 16:16
	int y1_rcp = rcp((y1-y)<<8); // 16:16
	
	int x0_delta = (s64(x0-x) * y0_rcp)>>16; // 24:8
	int x1_delta = (s64(x1-x) * y1_rcp)>>16; // 24:8
	
	int y_hi_offs = (dst_y<<8) + 0x40 - y;
	int y_lo_offs = (dst_y<<8) - 0x40 - y;
	
	int cmp_x = ((x+0x7f)>>8)<<8;
	int x0_hi = x + ((x0_delta*y_hi_offs)>>8);
	int x0_lo = x + ((x0_delta*y_lo_offs)>>8);
	int x1_hi = x + ((x1_delta*y_hi_offs)>>8);
	int x1_lo = x + ((x1_delta*y_lo_offs)>>8);

	
	// evaluate inside test for both edge functions at 4 sample points
	bool cw0 = y0>y;
	bool cw1 = y1<y;
	bool in0_0 = (x0_hi<(cmp_x-0x40))==cw0;
	bool in0_1 = (x0_hi<(cmp_x+0x40))==cw0;
	bool in0_2 = (x0_lo<(cmp_x-0x40))==cw0;
	bool in0_3 = (x0_lo<(cmp_x+0x40))==cw0;
	bool in1_0 = (x1_hi<(cmp_x-0x40))==cw1;
	bool in1_1 = (x1_hi<(cmp_x+0x40))==cw1;
	bool in1_2 = (x1_lo<(cmp_x-0x40))==cw1;
	bool in1_3 = (x1_lo<(cmp_x+0x40))==cw1;

	// count sample points that are inside both edge functions
	int covg=0;
	if (in0_0 && in1_0) covg+=0x4000;
	if (in0_1 && in1_1) covg+=0x4000;
	if (in0_2 && in1_2) covg+=0x4000;
	if (in0_3 && in1_3) covg+=0x4000;
	dprintf("%i\n", covg>>14);
	
///	int covg0 = (x0_hi<cmp_x) + (x0_lo<cmp_x);
//	int covg1 = (x1_hi<cmp_x) + (x1_lo<cmp_x);
//	if (y0>y) covg0 = 2-covg0;
//	if (y1>y) covg1 = 2-covg1;
//	int covg = (covg0+covg1)*(0x10000/4);
	
	//int fg = 

	s32 val=(0x10000-covg)>>8;
	if (val>254) val=254;	
	
	val = read_pixel(fb+dst_x+dst_y*GBA_WIDTH);
	val = min(255, val+80);
	write_pixel(fb+dst_x+dst_y*GBA_WIDTH, val);	
//	dprintf(" %i, ..  %i, %i, %i, %i\n", cmp_x-x, x0_hi<cmp_x, x1_hi<cmp_x, x0_lo<cmp_x, x1_lo<cmp_x);	
//	int x0_hi = x0 + (x0_delta*(y_hi-y))
}
*/
#if 0
// NOTE: It is essential that precision everywhere is consistent with that in texturemapper.
void antialias_edge(s32 x0, s32 y0, s32 x1, s32 y1, u8* fb)
{	
	const int clip_top=0;
	const int clip_bottom=160;
				
	int w = (x1-x0);
	int h = (y1-y0);
	if (w<0) w=-w;
	if (h<0) h=-h;
	
	
	
	
	if (y1<y0) {
		swap(y0,y1);
		swap(x0,x1);
	}
	
	int vert_xl = (x0+0xff)>>8;
	int vert_xr = (x1+0xff)>>8;
	int vert_yt = (y0+0x7f)>>8;
	int vert_yb = (y1+0x7f)>>8;

//	write_pixel(fb+vert_xl+vert_yt*GBA_WIDTH, 0);

	if (vert_xl>vert_xr) swap(vert_xl, vert_xr);
	
	s32 y_rcp  = rcp((y1-y0)<<8); // 16:16
	s32 x_rcp  = rcp((x1-x0)<<8); // 16:16

	SlopeIter xiter;
	xiter.delta = (s64(x1-x0) * y_rcp)>>8; // 16:16
	if ((xiter.delta>>16) >  256) xiter.delta = 256<<16;
	if ((xiter.delta>>16) < -256) xiter.delta =-256<<16;
	xiter.v = x0<<8; // 16:16

	SlopeIter yiter;	
	yiter.delta = (s64(y1-y0) * x_rcp)>>8; // 16:16
	if (yiter.delta<0) yiter.delta=-yiter.delta;
	if (yiter.delta>0x10000) yiter.delta=0x10000;
//	yiter.v = y0<<8; // 16:16

	int top		= (y0+127)>>8;
	int bottom	= (y1+256+127)>>8;
//	if (xiter.delta<0) {
//		top++;
//		bottom++;
//	}
	
	if (top>=clip_bottom || bottom<=clip_top) return;

	// clipping
	if (bottom>clip_bottom) bottom=clip_bottom;
	if (top<clip_top) top=clip_top;
	xiter.roundup(y0, top<<8);

	// roundup the y iterator
//	yiter.roundup16(xiter.v);


	u8* dst = &fb[top*GBA_WIDTH];
	{
		int abs_delta = xiter.delta;
		if (abs_delta<0) abs_delta=-abs_delta;



		for (int y=top; y<bottom; y++) {
		
			int xl16 = xiter.v - (abs_delta>>1);
			int xl = (xl16+0xFFFF)>>16;     // på venstre side av xl er den 0
			int xm = (xiter.v+0xFFFF)>>16;  // på venstre side av xm er den 0.5
			int xr = (xl16+abs_delta+0xFFFF)>>16; // på venstre side av xr er den 1

			if (xr<=xl) xr=xl+1;
						
			if (xl<0) xl=0;
			if (xr>240) xr=240;

			if (y<=vert_yt) {
				if (xiter.delta>=0 && xl<vert_xl+1) xl=vert_xl+1;
				if (xiter.delta<=0 && xr>vert_xr)   xr=vert_xr;
			} 
			if (y>=vert_yb) {
				if (xiter.delta<=0 && xl<vert_xl+1) xl=vert_xl+1;
				if (xiter.delta>=0 && xr>vert_xr)   xr=vert_xr;
			}

							
			yiter.v = 0;
			yiter.roundup16(xl16, xl<<16);
			
			for (int x=xl; x<xr; x++) {

				// fg=low right
				int fg, bg; 
				if (w>=h) {
					if ( x<xm ) {
						if ( xiter.delta<=0 ) {
							bg = read_pixel(dst+x);
							fg = read_pixel(dst+x+GBA_WIDTH);
						} else {
							fg = read_pixel(dst+x-GBA_WIDTH);
							bg = read_pixel(dst+x);
						}
					} else {
						if ( xiter.delta<=0 ) {
							bg = read_pixel(dst+x-GBA_WIDTH);
							fg = read_pixel(dst+x);
						} else {
							fg = read_pixel(dst+x);
							bg = read_pixel(dst+x+GBA_WIDTH);
						}
					}
				} else {
					fg = read_pixel(dst+xr);
					bg = read_pixel(dst+xl-1);
				}

				int covg = yiter.v;
				s32 val = blend(bg, fg, covg);

				//val = min(254,read_pixel(dst+x)+40);
				//val=(covg)>>8;
				write_pixel(dst+x, val);

				yiter.increment();
			}
							
			xiter.increment();
			dst+=GBA_WIDTH;		
		}
	}	
/*	
	// draw
	if (true || w>h) {
		int abs_delta = xiter.delta;
		if (abs_delta<0) abs_delta=-abs_delta;
		if (bottom>clip_bottom-1) bottom=clip_bottom-1;

		SlopeIter cIter;
		if ((abs_delta>>16) > 256) abs_delta=256<<16;
		cIter.delta = rc(xiter.delta);
		if (cIter.delta<-0x10000) cIter.delta=-0x10000;
		if (cIter.delta> 0x10000) cIter.delta= 0x10000;

		for (int y=top; y<bottom; y++) {
		
			int xl16 = xiter.v - (abs_delta/2);
			int xl = (xl16+0x7FFF)>>16;     // på venstre side av xl er den 0
			int xm = (xiter.v+0xFFFF)>>16;  // på venstre side av xm er den 0.5
			int xr = (xl16+abs_delta + 0x7FFF)>>16; // på venstre side av xr er den 1
			if (xl16<0) {
				xl16=0;
				xl=0;
			}		
			if (xr==xl)xr++;
			if (xr>240) xr=240;
							
			cIter.v = (cIter.delta>0) ? 0 : 0x10000;//(xiter.delta>0) ? 0x10000 : 0;
			cIter.roundup16( xl16-0x8000 );
			if (w>h) {
			} else {
//				cIter.v -= (xl16-0x8000)&0xFFFF;
			}
			for (int x=xl; x<xr; x++) {
				int offs = 0;
				if ( (x>=xm) == (xiter.delta>=0) ) offs=GBA_WIDTH;
				
				int bg = 0x80;//read_pixel(dst+x+offs-GBA_WIDTH);
				int fg = 0;//read_pixel(dst+x+offs);

				int covg = cIter.v;
 				if (covg>0x10000) covg=0x10000;
				if (covg<0) covg=0;

				s32 val = blend(fg, bg, covg)&0xFF;

//val = read_pixel(dst+x)+50;
//if (val>255)val=255;
				write_pixel(dst+x, val);

				cIter.increment();
			}
							
			xiter.increment();
			dst+=GBA_WIDTH;		
		}
	} else {
		if (bottom>clip_bottom) bottom=clip_bottom;
		for (int y=top; y<bottom; y++) {
			int fg_x  = (xiter.v+0xFFFF)>>16;
			int bg_x  = fg_x-1;
			int dst_x = (xiter.v+0x8000)>>16;
			
			s32 covg = 0x10000-((xiter.v-0x8000)&0xFFFF);
			 
			if (dst_x>=0 && dst_x<GBA_WIDTH) {
				s32 bg = read_pixel(dst+bg_x);
				s32 fg = read_pixel(dst+fg_x);
				
				s32 val = blend(bg, fg, covg);
					
				write_pixel(dst+dst_x, val);
			}
			
			xiter.increment();
			dst+=GBA_WIDTH;		
		}
	}*/
}
#elif 0

// NOTE: It is essential that precision everywhere is consistent with that in texturemapper.
void antialias_edge(s32 x0, s32 y0, s32 x1, s32 y1, u8* target)
{	
	const int clip_top=0;
	const int clip_bottom=160;
	
	if (y0>y1) {
		swap(y0,y1);
		swap(x0,x1);
	}
				
	bool horizontal;
	{ 
		int w = (x1-x0);
		int h = (y1-y0);
		if (w<0) w=-w;
		if (h<0) h=-h;
		horizontal = w>h;
	}
		
	// rasterization must be identical to polyfiller.
	// modifying top/bottom is safe since they are integers
	// so prestepping with them gives same result as iteration.
	int top,bottom;
	int x_low, x_high;
	if (horizontal) {
		// HORIZONTAL: x_low/x_high restrict line
		
		// half pixel extended border in y to make room 
		top		= (y0+255-128)>>8;
		bottom	= (y1+255+127)>>8;
		
		// remove half pixel for endpoints in x
		x_low  = (min(x0, x1)+255+127)>>8;
		x_high = (max(x0, x1)+255-128)>>8;
	} else {
		// VERTICAL CASE: top/bottom restrict line
		
		// remove a half pixel for endpoints in y
		top		= (y0+255+127)>>8;
		bottom	= (y1+255-128)>>8;
		
		// trivial x_low, x_high
		x_low=0;
		x_high=GBA_WIDTH-1;
	}

	// y culling
	if (top>=clip_bottom || bottom<=clip_top) return;

	// clipping
	if (top<clip_top) top = clip_top; // +y clipping
	if (bottom>clip_bottom) bottom = clip_bottom; // +y clipping
	if (x_low<0) x_low=0;
	if (x_high>GBA_WIDTH-1) x_high=GBA_WIDTH-1;
	

	// divisions for slopes
	s32 h_rcp  = urcp((u32)(y1 - y0) << 8);	// 16:16

	// init long x iterator
  	Iter xIter;
	xIter.delta = (s64(x1 - x0) * h_rcp)>>8; // 16:16
	xIter.v = (x0<<8) + 0xFFFF; // 16:16

	// prestep to top
	xIter.increment((top<<8)-y0);

	// render
	int absDelta = xIter.delta;
	if (absDelta<0) absDelta=-absDelta;

	u8* dst = &target[top*GBA_WIDTH];
	
	for (int y=top; y<bottom; y++) { 		
		int xl, xr;
		if (horizontal) {
			// disse er dobbla. de blir halvert og avrunda etter sortering under.
			int xStart = (xIter.v*2)-xIter.delta; 
			int xStop  = (xIter.v*2)+xIter.delta;

			xl=xStart;
			xr=xStop;
			if (xr<xl) swap(xl,xr);
			// round down to 16 bits
			xl = xl>>17;
			xr = (xr+1)>>17;
			// clip
			if (xl<=x_low)  xl = x_low; 
			if (xr>=x_high) xr = x_high;
			// y er korrekt på x == Xiter.v
		} else {
			xl=(xIter.v-0x7FFF)>>16;
			xr=(xIter.v+0x8000)>>16;
			// x er korrekt og y er korrekt
		}
		
		for (int x=xl; x<xr; x++) {
			// draw
			int val = read_pixel(dst+x);
			val = min(255, val+80);
			write_pixel(dst+x, val);
		}
		
		// increment
		xIter.increment();
		dst += GBA_WIDTH;
	}

}
#else
// NOTE: It is essential that precision everywhere is consistent with that in texturemapper.
void antialias_edge(s32 x0, s32 y0, s32 x1, s32 y1, u8* target, const u8* shadeTable)
{	
	const int clip_top=0;
	const int clip_bottom=160;

	// x culling
	if (max(x0, x1)<0 || min(x0, x1)>=(GBA_WIDTH<<8)) return;

	if (y0>y1) {
		swap(y0,y1);
		swap(x0,x1);
	}
				
	bool horizontal;
	{ 
		int w = (x1-x0);
		int h = (y1-y0);
		if (w<0) w=-w;
		if (h<0) h=-h;
		horizontal = w>h;
	}
		
	// rasterization must be identical to polyfiller.
	// modifying top/bottom is safe since they are integers
	// so prestepping with them gives same result as iteration.
	int top,bottom;
	int x_low, x_high;
	if (horizontal) {
		// HORIZONTAL: x_low/x_high restrict line
		
		// half pixel extended border in y to make room 
		top		= (y0+255-128)>>8;
		bottom	= (y1+255+127)>>8;
		
		// remove half pixel for endpoints in x
		x_low  = (min(x0, x1)+255+127)>>8;
		x_high = (max(x0, x1)+255-128)>>8;
	} else {
		// VERTICAL CASE: top/bottom restrict line
		
		// remove a half pixel for endpoints in y
		top		= (y0+255+127)>>8;
		bottom	= (y1+255-128)>>8;
		
		// trivial x_low, x_high
		x_low=0;
		x_high=GBA_WIDTH-1;
	}

	// y culling
	if (top>=clip_bottom || bottom<=clip_top) return;


	// clipping
	if (top<clip_top) top = clip_top; // +y clipping
	if (bottom>clip_bottom) bottom = clip_bottom; // +y clipping
	if (x_low<0) x_low=0;
	if (x_high>GBA_WIDTH-1) x_high=GBA_WIDTH-1;
	

	// divisions for slopes
	s32 h_rcp  = urcp((u32)(y1 - y0) << 8);	// 16:16

	// init long x iterator
  	Iter xIter;
	xIter.delta = (s64(x1 - x0) * h_rcp)>>8; // 16:16
	xIter.v = (x0<<8) + 0xFFFF; // 16:16

	// prestep to top
	xIter.increment((top<<8)-y0);

	// render
	int absDelta = xIter.delta;
	if (absDelta<0) absDelta=-absDelta;

	u8* dst = &target[top*GBA_WIDTH];
	
	if (horizontal) {
		s32  w_rcp  = rcp((x1 - x0) << 8);	// 16:16
		Iter yIter;
		yIter.delta = (s64(y1 - y0) * w_rcp)>>8;
		//yIter.v = (y0<<8)+0x7f00;
//		if (xIter.delta<0) yIter.delta = -yIter.delta;
		for (int y=top; y<bottom; y++) { 		
			int xStart = (xIter.v*2)-xIter.delta; 
			int xStop  = (xIter.v*2)+xIter.delta;
			int xl=xStart;
			int xr=xStop;
			if (xr<xl) swap(xl,xr);
			xl = xl>>17;
			xr = (xr+1)>>17;

			// clip
			if (xl<=x_low)  xl = x_low; 
			if (xr>=x_high) xr = x_high;
			int xm = xIter.v>>16;

			// FIXME
			// y er eksakt 0.5 på x == Xiter.v
			// så, prestep den til xl.
			// HVORFOR ADDE 1??
			s64 step = (xl<<16) + 0x10000 - xIter.v;
			yIter.v = 0x8000 + ((step*yIter.delta)>>16);

			// do the iteration on x
			// FIXME: optimize
			for (int x=xl; x<xr; x++) {
				int covg = (yIter.v&0xFFFF);
				int sample_offs = 0;
				if (  (xIter.delta<0) == (x>=xm)  ) sample_offs=-GBA_WIDTH;
				int bg = read_pixel(dst+x+sample_offs+GBA_WIDTH);
				int fg = read_pixel(dst+x+sample_offs);
				int val = blend(covg, fg, bg, shadeTable);

				write_pixel(dst+x, val);

				yIter.increment();
			}
			
			// increment
			xIter.increment();
			dst += GBA_WIDTH;
		}
	} else {
		int y=top;
		// loop until we are in the screen
		for (;y<bottom; y++) {
			// test if we are inside
			int xm = xIter.v>>16;
			if (xm>=1 && xm<GBA_WIDTH) break;
			// increment
			xIter.increment();
			dst += GBA_WIDTH;
		}
		// NOW, do the drawing
		for (; y<bottom; y++) { 		
			int x  = (xIter.v-0x7FFF)>>16;
			int xm = xIter.v>>16;

			// exit when we are outside of the screen
			if ( !(xm>=1 && xm<GBA_WIDTH)) break;

			// xIter.v er korrekt og y er korrekt.
			// det skal da gå an å bruke xIter.v til å blende direkte!
			int bg = read_pixel(dst+xm);
			int fg = read_pixel(dst+xm-1);
			int covg = (xIter.v-0x7FFF)&0xFFFF;
			int val = blend(covg, fg, bg, shadeTable);
			write_pixel(dst+x, val);

			// increment
			xIter.increment();
			dst += GBA_WIDTH;
		}
	}

}

#endif
