#include "poly.h"
#include "support.h"
#include "math.h"
#include "exstructs.h"
#include "base.h"

#define LEFT  1
#define RIGHT 2

#define FILLER_SHIFT256_1X1
#define FILLER_TEX256_1X1
#define FILLER_TEX256_2X1
#define FILLER_NMAP_1X1
//#define FILLER_NMAP_2X1
//#define FILLER_EBUMP_1X1
//#define FILLER_EBUMP_2X1


namespace {

	// functors for the different fillers





	// hq hline routine




	// draw_part
/*	struct Varying {
		s32 dvdx, dvdy;
		u32 vL;
	};
	
	class Shader {
	
		Varying u, v;
	};

	template<class T>
	void drawPartHq<T>(T shader);

	template<class T>
	void drawPartLq<T>(T shader);
*/

	// stuff used by draw_part
	const s16* global_light;

	void hline_shift256_1x1(u8* dst, int x1, int x2, u32 u, u32 v, s32 duvdx, s32 dudx, s32 dvdx, const u8* tex0, const u8* tex1, int palOffs) {
#ifndef FILLER_SHIFT256_1X1
		assert(0);
#else
		const u8* tex = tex0;
		
#define ITERATION {\
	u32 bg = *(u16*)dst; \
	u32 index0 = (uv>>24)|(uv&0xFF00);\
	u32 col0 = tex[index0];\
	uv += duvdx;\
	u32 index1 = (uv>>24)|(uv&0xFF00);\
	u32 col1 = tex[index1];\
	uv += duvdx;\
	*(u16*)dst = ((bg & 0xFF) >> col0) | (((bg & 0xFF00) >> col1) & 0xFF00) ;\
	dst += 2;\
}

		if (x1&1) {
			u32 index = (u>>24)|((v>>16)&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			u8 col = tex[index];
			val = (val&0x00FF) | (((val & 0xFF00) >> col) & 0xFF00);
			x1++;
			u += dudx;
			v += dvdx;
		}


		dst += x1;
		s32 counter = (x2-x1)>>1;
		register unsigned loopCount = counter>>3;
		const int preCount = counter&7;
		u32 uv = (u&0xFFFF0000)|u16(v>>16);
		u+=dudx*preCount*2;
		v+=dvdx*preCount*2;
		switch (preCount) {
			do {
			uv = (u&0xFFFF0000)|u16(v>>16);
			u+=dudx*16;
			v+=dvdx*16;
			ITERATION;
			case 7:  ITERATION;
			case 6:  ITERATION;
			case 5:  ITERATION;
			case 4:  ITERATION;
			case 3:  ITERATION;
			case 2:  ITERATION;
			case 1:  ITERATION;
			case 0:;  
			} while (loopCount--);
		}
#undef ITERATION
		if (x2&1) {
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = *((u16*)dst);
			u8 col = tex[index];
			val = (val&0xFF00) | ((val & 0xFF) >> col);
			x1++;
		}
#endif
	}


	void hline_tex256_2x1(u8* dst, int x1, int x2, u32 u, u32 v, s32 duvdx, s32 dudx, s32 dvdx, const u8* tex, const u8* tex1, int palOffs) {
#ifndef FILLER_TEX256_2X1
		assert(0);
#else
#define ITERATION {\
	u32 index = (uv>>24)|(uv&0xFF00);\
	u8 col = tex[index];\
	*dst = col;\
	dst += 2;\
	uv += (duvdx<<1); \
}


		if (x1&1) {
			u32 index = (u>>24)|((v>>16)&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			u8 col = tex[index];
			val = (val&0x00FF)|(u32(col)<<8);
			x1++;
			u += dudx;
			v += dvdx;
		}


		dst += x1;
		s32 counter = (x2-x1)>>1;
		u32 uv = (u&0xFFFF0000)|u16(v>>16);
#if 1
		while (counter>=8) {
			ITERATION;	
			ITERATION;	
			ITERATION;	
			ITERATION;	
			ITERATION;	
			ITERATION;	
			ITERATION;	
			ITERATION;	
			counter-=8;
			u+=dudx*16;
			v+=dvdx*16;
			uv = (u&0xFFFF0000)|u16(v>>16);
		}
//		while (counter--) ITERATION;
		switch (counter)
		{
			case 7:  ITERATION;
			case 6:  ITERATION;
			case 5:  ITERATION;
			case 4:  ITERATION;
			case 3:  ITERATION;
			case 2:  ITERATION;
			case 1:  ITERATION;
		}
#else	
		register unsigned loopCount = counter>>3;
		const int preCount = counter&7;
		u+=dudx*preCount*2;
		v+=dvdx*preCount*2;
		switch (preCount) {
			do {
			uv = (u&0xFFFF0000)|u16(v>>16);
			u+=dudx*16;
			v+=dvdx*16;
			ITERATION;
			case 7:  ITERATION;
			case 6:  ITERATION;
			case 5:  ITERATION;
			case 4:  ITERATION;
			case 3:  ITERATION;
			case 2:  ITERATION;
			case 1:  ITERATION;
			case 0:;  
			} while (loopCount--);
		}
#endif
#undef ITERATION
		if (x2&1) {
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = *((u16*)dst);
			u8 col = tex[index];
			val = (val&0xFF00)|col;
			x1++;
		}
#endif
	}


	void hline_tex256_1x1(u8* dst, int x1, int x2, u32 u, u32 v, s32 duvdx, s32 dudx, s32 dvdx, const u8* tex0, const u8* tex1, int palOffs) {
#ifndef FILLER_TEX256_1X1
		assert(0);
#else
		const u8* tex = tex0;
		
#define ITERATION {\
	u32 index0 = (uv>>24)|(uv&0xFF00);\
	u32 col0 = tex[index0];\
	uv += duvdx;\
	u32 index1 = (uv>>24)|(uv&0xFF00);\
	u32 col1 = tex[index1];\
	uv += duvdx;\
	*(u16*)dst = (col1<<8) | col0;\
	dst += 2;\
}

		if (x1&1) {
			u32 index = (u>>24)|((v>>16)&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			u8 col = tex[index];
			val = (val&0x00FF)|(u32(col)<<8);
			x1++;
			u += dudx;
			v += dvdx;
		}


		dst += x1;
		s32 counter = (x2-x1)>>1;
		register unsigned loopCount = counter>>3;
		const int preCount = counter&7;
		u32 uv = (u&0xFFFF0000)|u16(v>>16);
/*
		if (x1<0) {
			uv -= ((duvdx&0xFFFF0000)*x1)|u16(duvdx*x1);
			x1=0;
		}
*/
		u+=dudx*preCount*2; 
		v+=dvdx*preCount*2;

		switch (preCount) {
			do {
			uv  = u & 0xFFFF0000;
			uv |= u32(v) >> 16;
			u+=dudx*16;
			v+=dvdx*16;
			ITERATION;
			case 7:  ITERATION;
			case 6:  ITERATION;
			case 5:  ITERATION;
			case 4:  ITERATION;
			case 3:  ITERATION;
			case 2:  ITERATION;
			case 1:  ITERATION;
			case 0:;  
			} while (loopCount--);
		}
#undef ITERATION
		if (x2&1) {
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = *((u16*)dst);
			u8 col = tex[index];
			val = (val&0xFF00)|col;
			x1++;
		}
#endif
	}



	void hline_nmap_2x1(u8* dst, int x1, int x2, u32 u, u32 v, s32 duvdx, s32 dudx, s32 dvdx, const u8* tex0, const u8* tex1, int palOffs) {
#ifndef FILLER_NMAP_2X1
		assert(0);
#else
		const s16* tex = (s16*)tex0;
		s32 lightx = global_light[0];
		s32 lighty = global_light[1];
		s32 lightz = global_light[2];


#define STUFF(ut, inn) {\
	s32 lol = 0; \
	lol += lightx * (s32(inn<<16)>>27);\
	lol += lighty * (s32(inn<<21)>>27);\
	lol += lightz * (s32(inn<<26)>>27);\
	lol >>= 8;\
	ut = lol+palOffs;\
}


#define ITERATION {\
	u32 index = (uv>>24)|(uv&0xFF00);\
	s32 val = tex[index];\
	STUFF(*dst, val);\
	dst += 2;\
	uv += (duvdx<<1); \
}

#define ITERATION_ASM \
	asm (							\
    "and r0, %0, #65280\n\t"		\
	"orr r0, r0, %0, lsr #24\n\t"	\
	"mov r0, r0, asl #1\n\t"		\
	"ldrsh r1, [r0, %2]\n\t"		\
	"mov r2, r1, asr #11\n\t"		\
	"mul r0, %3, r2\n\t"			\
	"mov r2, r1, asl #21\n\t"		\
	"mov r2, r2, asr #27\n\t"		\
	"mla r0, %4, r2, r0\n\t"		\
	"mov r2, r1, asl #26\n\t"		\
	"mov r2, r2, asr #27\n\t"		\
	"mla r0, %5, r2, r0\n\t"		\
	"add r0, %6, r0, asr #8\n\t"	\
	"strb r0, [%1], #2\n\t"			\
	"add %0, %0, %7"				\
		: "=r"(uv), "=r"(dst)		\
		: "r"(tex), "r"(lightx), "r"(lighty), "r"(lightz), "r"(palOffs), "r"(duvdx<<1), "0"(uv), "1"(dst) \
		: "r0", "r1", "r2"			\
		)




		if (x1&1) {
			u32 index = (u>>24)|((v>>16)&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			s32 col = tex[index];
			STUFF(col, col);
			val = (val&0x00FF)|(u32(col)<<8);
			x1++;
			u += dudx;
			v += dvdx;
		}


		dst += x1;
		s32 counter = (x2-x1)>>1;
		register unsigned loopCount = counter>>3;
		const int preCount = counter&7;
		u32 uv = (u&0xFFFF0000)|u16(v>>16);
		u+=dudx*preCount*2;
		v+=dvdx*preCount*2;
		switch (preCount) {
			do {
				uv = (u&0xFFFF0000)|u16(v>>16);
				u+=dudx*32;
				v+=dvdx*32;
				ITERATION;
				case 7:  ITERATION;
				case 6:  ITERATION_ASM;
				case 5:  ITERATION;
				case 4:  ITERATION;
				case 3:  ITERATION;
				case 2:  ITERATION;
				case 1:  ITERATION;
				case 0:;  
			} while (loopCount--);
		}
		if (x2&1) {
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = *((u16*)dst);
			s32 col = tex[index];
			STUFF(col, col);
			val = (val&0xFF00)|col;
			x1++;
		}
#undef STUFF
#undef ITERATION
#undef ITERATION_ASM
#endif
	}



	void hline_nmap_1x1(u8* dst, int x1, int x2, u32 u, u32 v, s32 duvdx, s32 dudx, s32 dvdx, const u8* tex0_8, const u8* tex1, int palOffs) {
#ifndef FILLER_NMAP_1X1
		assert(0);
#else
		const s16* tex = (s16*)tex0_8;
		s32 lightx = global_light[0];
		s32 lighty = global_light[1];
		s32 lightz = global_light[2];


#define STUFF(ut, inn) {\
	s32 lol = 0; \
	lol += lightx * (s32(inn<<16)>>27);\
	lol += lighty * (s32(inn<<21)>>27);\
	lol += lightz * (s32(inn<<26)>>27);\
	lol >>= 8;\
	ut = lol+palOffs;\
}


#define ITERATION {\
	u32 index0 = (uv>>24)|(uv&0xFF00);\
	s32 tmp0 = tex[index0];\
	u32 col0;\
	STUFF(col0, tmp0);\
	uv += duvdx;\
	u32 index1 = (uv>>24)|(uv&0xFF00);\
	s32 tmp1 = tex[index1];\
	u32 col1;\
	STUFF(col1, tmp1);\
	uv += duvdx;\
	*(u16*)dst = (col1<<8)|col0;\
	dst += 2;\
}


		if (x1&1) {
			u32 index = (u>>24)|((v>>16)&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			s32 col = tex[index];
			STUFF(col, col);
			val = (val&0x00FF)|(u32(col)<<8);
			x1++;
			u += dudx;
			v += dvdx;
		}


		dst += x1;
		s32 counter = (x2-x1)>>1;
		register unsigned loopCount = counter>>3;
		const int preCount = counter&7;
		u32 uv = (u&0xFFFF0000)|u16(v>>16);
		u+=dudx*preCount*2;
		v+=dvdx*preCount*2;
		switch (preCount) {
			do {
				uv = (u&0xFFFF0000)|u16(v>>16);
				u+=dudx*16;
				v+=dvdx*16;
				ITERATION;
				case 7:  ITERATION;
				case 6:  ITERATION;
				case 5:  ITERATION;
				case 4:  ITERATION;
				case 3:  ITERATION;
				case 2:  ITERATION;
				case 1:  ITERATION;
				case 0:;  
			} while (loopCount--);
		}
		if (x2&1) {
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = *((u16*)dst);
			s32 col = tex[index];
			STUFF(col, col);
			val = (val&0xFF00)|col;
			x1++;
		}
#undef STUFF
#undef ITERATION
#endif
	}



	void hline_ebump_2x1(u8* dst, int x1, int x2, u32 u, u32 v, s32 duvdx, s32 dudx, s32 dvdx, const u8* tex0_8, const u8* tex1, int palOffs) {
#ifndef FILLER_EBUMP_2X1
		assert(0);
#else
		const s16* tex = (s16*)tex0_8;
		s32 lightangle = -global_light[0];
		



#define STUFF(ut, inn) {\
	int v = ((inn-lightangle)<<1)&0xFF00; \
	int u = inn & 0xFF; \
	ut = tex1[v|u]; \
}


		//u32 index = (uv>>24)|(uv&0xFF00);
#define ITERATION {\
	u32 index0 = (uv>>24)|(uv&0xFF00);\
	u32 val0 = tex[index0];\
	STUFF(*dst, val0);\
	dst += 2;\
	uv += (duvdx<<1); \
}


		if (x1&1) {
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			u16 col = tex[index];
			STUFF(col, col);
			val = (val&0x00FF)|(u32(col)<<8);
			x1++;
			uv += duvdx;
		}

		dst += x1;
		s32 counter = (x2-x1)>>1;
		register unsigned c = counter >> 3;
		switch (counter&7) {
			do {
			ITERATION;
			case 7:  ITERATION;
			case 6:  ITERATION;
			case 5:  ITERATION;
			case 4:  ITERATION;
			case 3:  ITERATION;
			case 2:  ITERATION;
			case 1:  ITERATION;
			case 0:;  
			} while (c--);
		}
#undef ITERATION
		if (x2&1) {
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = *((u16*)dst);
			u16 col = tex[index];
			STUFF(col, col);
			val = (val&0xFF00)|col;
			x1++;
		}
#endif
	}



	void hline_ebump_1x1(u8* dst, int x1, int x2, u32 u, u32 v, s32 duvdx, s32 dudx, s32 dvdx, const u8* tex0, const u8* tex1, int palOffs) {
#ifndef FILLER_EBUMP_1X1
		assert(0);
#else
		const s16* tex = (s16*)tex0;
		s32 lightangle = -global_light[0];
		



#define STUFF(ut, inn) {\
	int v = ((inn-lightangle)<<1)&0xFF00; \
	int u = inn & 0xFF; \
	ut = tex1[v|u]; \
}


		//u32 index = (uv>>24)|(uv&0xFF00);
#define ITERATION {\
	u32 index0 = (uv>>24)|(uv&0xFF00);\
	u32 tmp0 = tex[index0];\
	u32 col0;\
	STUFF(col0, tmp0);\
	uv += duvdx;\
	u32 index1 = (uv>>24)|(uv&0xFF00);\
	u32 tmp1 = tex[index1];\
	u32 col1;\
	STUFF(col1, tmp1);\
	uv += duvdx;\
	*(u16*)dst = (col1<<8) | col0;\
	dst += 2;\
}



		if (x1&1) {
			u32 index = (u>>24)|((v>>16)&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			u16 col = tex[index];
			STUFF(col, col);
			val = (val&0x00FF)|(u32(col)<<8);
			x1++;
			u += dudx;
			v += dvdx;
		}


		dst += x1;
		s32 counter = (x2-x1)>>1;
		register unsigned loopCount = counter>>3;
		const int preCount = counter&7;
		u32 uv = (u&0xFFFF0000)|u16(v>>16);
		u+=dudx*preCount*2;
		v+=dvdx*preCount*2;
		switch (preCount) {
			do {
			uv = (u&0xFFFF0000)|u16(v>>16);
			u+=dudx*16;
			v+=dvdx*16;
			ITERATION;
			case 7:  ITERATION;
			case 6:  ITERATION;
			case 5:  ITERATION;
			case 4:  ITERATION;
			case 3:  ITERATION;
			case 2:  ITERATION;
			case 1:  ITERATION;
			case 0:;  
			} while (loopCount--);
		}
#undef ITERATION
		if (x2&1) {
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = *((u16*)dst);
			u16 col = tex[index];
			STUFF(col, col);
			val = (val&0xFF00)|col;
			x1++;
		}
#endif
	}




	struct XIter {
		s32 x;
		s32 delta_x;
		INLINE void increment() {
			x+=delta_x;
		}
		INLINE void increment(int count) {
			x+=(s64(delta_x)*count)>>8;
		}
	};


	struct UvIter {
	private:
		s32 delta_u;
		s32 delta_v;
	public:
		u32 u, v;
		INLINE void increment()
		{
			u += delta_u;
			v += delta_v;
		}
		
		INLINE void increment(s32 count) // 24:8
		{
			u += (s64(delta_u) * count)>>8;
			v += (s64(delta_v) * count)>>8;
		}
		
		
		INLINE s32 setUv(u32 u, u32 v) // 8:24
		{
			this->u = u;
			this->v = v;
		}
		
		INLINE s32 setUvDelta(s32 u_delta, s32 v_delta) // 8:24
		{
			this->delta_u = u_delta;
			this->delta_v = v_delta;
		}
	};




	typedef void (*HLinePtr) (u8* dst, int x1, int x2, u32 u, u32 v, s32 duvdx, s32 dudx, s32 dvdx, const u8* tex0, const u8* tex1, int palOffs);


	INLINE void draw_part(u8* dst, u8* dstStop, UvIter& uvIter, XIter& xIterL, XIter& xIterR, s32 duvdx, s32 dudx, s32 dvdx, HLinePtr hLinePtr, const Material& mtl) {
		while(dst<dstStop) {
			int xl = xIterL.x>>16;
			int xr = xIterR.x>>16;

			// clip and prestep
			int l_prestep = 0;
			if (xr > GBA_WIDTH) xr=GBA_WIDTH;
			if (xl < 0) xl = 0;
						
			// draw
			if (xr > xl) {
				u32 u = uvIter.u + (xl*dudx);
				u32 v = uvIter.v + (xl*dvdx);
				hLinePtr(dst, xl, xr, u, v, duvdx, dudx, dvdx, mtl.tex[0], mtl.tex[1], mtl.palOffs);
			}
			
			// increment
			xIterL.increment();
			xIterR.increment();
			uvIter.increment();
			dst += GBA_WIDTH;
		}
	}

	const unsigned char conf_tab[8*8] = {
		0,		255,	255,	255,	255,	255,	255,	255,
		RIGHT,	2,		1,		0,		1,		0,		255,	255,
		RIGHT,	1,		0,		2,		0,		2,		255,	255,
		LEFT,	1,		2,		0,		0,		2,		255,	255,
		RIGHT,	0,		2,		1,		2,		1,		255,	255,
		LEFT,	2,		0,		1,		1,		0,		255,	255,
		LEFT,	0,		1,		2,		2,		1,		255,	255,
		0,		255,	255,	255,	255,	255,	255,	255,
	};

}; // namespace


  

void polyfiller(u8* target, PolyVertex v[3], const Material& mtl, const s16 light[3], int clip_top, int clip_bottom) {
	global_light = light;

	// get configuration
	unsigned int index = 0;
	if (v[0].y < v[1].y) index |= (1 << 2) * 8;
	if (v[1].y < v[2].y) index |= (1 << 1) * 8;
	if (v[2].y < v[0].y) index |= (1 << 0) * 8;
	int type						= conf_tab[index++];
	const PolyVertex& top_vert		= v[conf_tab[index++]];
	const PolyVertex& center_vert	= v[conf_tab[index++]];
	const PolyVertex& bottom_vert	= v[conf_tab[index++]];
	const PolyVertex& right_vert	= v[conf_tab[index++]];
	const PolyVertex& left_vert		= v[conf_tab[index++]];

	// vertical init
	int top		= (top_vert.y+255)>>8;
	int center	= (center_vert.y+255)>>8;
	int bottom	= (bottom_vert.y+255)>>8;

//	if (top>0) return;
//	if (v[0].x>0 && v[1].x>0 && v[2].x>0) return;
	// y culling
	if (top>=clip_bottom || bottom<=clip_top) return;

	// y clipping
	if (top<clip_top) top = clip_top; // +y clipping
	if (center<clip_top) center = clip_top; // +y clipping
	if (center>clip_bottom) center = clip_bottom; // +y clipping
	if (bottom>clip_bottom) bottom = clip_bottom; // +y clipping

	// select filler
	HLinePtr hLinePtr=NULL;
	switch(mtl.filler | (mtl.oneone<<7)) {
		default:
		case FILLER_TEX256:
			hLinePtr = (HLinePtr)hline_tex256_2x1;
			break;
		case FILLER_TEX256 | (1<<7):
			hLinePtr = (HLinePtr)hline_tex256_1x1;
			break;
		case FILLER_NMAP:
			hLinePtr = (HLinePtr)hline_nmap_2x1;
			break;
		case FILLER_NMAP | (1<<7):
			hLinePtr = (HLinePtr)hline_nmap_1x1;
			break;
		case FILLER_EBUMP:
			hLinePtr = (HLinePtr)hline_ebump_2x1;
			break;
		case FILLER_EBUMP | (1<<7):
			hLinePtr = (HLinePtr)hline_ebump_1x1;
			break;
		case FILLER_SHIFT | (1 << 7):
			hLinePtr = (HLinePtr)hline_shift256_1x1;
			break;
	}

	// check that sorting is correct
	ASSERT((bottom_vert.y - top_vert.y) >= 0);
	ASSERT((center_vert.y - top_vert.y) >= 0);
	ASSERT((bottom_vert.y - center_vert.y) >= 0);

	// divisions for slopes
	s32 long_rcp  = urcp((u32)(bottom_vert.y - top_vert.y) << 8);	// 16:16
	s32 upper_rcp = urcp((u32)(center_vert.y - top_vert.y) << 8);   // 16:16
	s32 lower_rcp = urcp((u32)(bottom_vert.y - center_vert.y) << 8);// 16:16
	s32 left_rcp  = (type==LEFT) ? upper_rcp : long_rcp;

	// init long x iterator
  	XIter xIter_long;
	xIter_long.delta_x = (s64(bottom_vert.x - top_vert.x) * long_rcp)>>8; // 16:16
	xIter_long.x = (top_vert.x<<8) + 0xFFFF; // 16:16

	// find uv deltas and init uv iterator
	UvIter uvIter;
	s32 dudx, dvdx;
	s32 duvdx;
	{
		s32 dX10 = (v[1].x-v[0].x)>>1; // 25:7
		s32 dX21 = (v[2].x-v[1].x)>>1;
		s32 dY10 = (v[1].y-v[0].y)>>1;
		s32 dY21 = (v[2].y-v[1].y)>>1;

		s32 area = (s64(dX21)*dY10 - s64(dX10)*dY21)>>10; // 28:4
		if (area<1) return;
		
		ASSERT(area >= 0);
		u32 area_inv = urcp((u32)area); // 4:28
//		u32 area_inv = 0xFFFFFFFFU / area;  // 4:28

		s32 du10 = (v[1].u-v[0].u)>>1; // 25:7
		s32 du21 = (v[2].u-v[1].u)>>1;
		s32 dv10 = (v[1].v-v[0].v)>>1;
		s32 dv21 = (v[2].v-v[1].v)>>1;

		dudx = ((du21*(dY10) - du10*(dY21)) * s64(area_inv))>>18; // 8:24
		dvdx = ((dv21*(dY10) - dv10*(dY21)) * s64(area_inv))>>18; // 8:24
		duvdx = (dudx & 0xFFFF0000) | (u32(dvdx)>>16); 

		s32 dudy = ((du10*(dX21)-du21*(dX10)) * s64(area_inv))>>18; // 8:24
		s32 dvdy = ((dv10*(dX21)-dv21*(dX10)) * s64(area_inv))>>18; // 8:24
		uvIter.setUvDelta(dudy, dvdy); // 8:24
		
		// find u and v at (x=0, top_vert.y)
		s32 u = s64(top_vert.u<<16) + ((s64(dudx)*-top_vert.x)>>8);
		s32 v = s64(top_vert.v<<16) + ((s64(dvdx)*-top_vert.x)>>8);
		uvIter.setUv(u, v);
	}


	// prestep to top for long iterators
	xIter_long.increment((top<<8)-top_vert.y);
	uvIter.increment((top<<8)-top_vert.y);


	// UPPER
	if (center>clip_top) { // upper is visible
		// prepare short x iterator
		XIter xIter_short;
		xIter_short.x = (top_vert.x<<8) + 0xFFFF; // 16:16
		xIter_short.delta_x = (s64(center_vert.x - top_vert.x) * upper_rcp)>>8;
		xIter_short.increment((top<<8)-top_vert.y);
		
		// render
		u8* dst = &target[top*GBA_WIDTH];
		u8* dstStop = &target[center*GBA_WIDTH];
		XIter& xIterL = (type==LEFT) ? xIter_short : xIter_long;
		XIter& xIterR = (type==LEFT) ? xIter_long  : xIter_short;
		draw_part(dst, dstStop, uvIter, xIterL, xIterR, duvdx, dudx, dvdx, hLinePtr, mtl);
	}

	// LOWER
	if (center<clip_bottom) { // lower is visible
		// prepare short x iterator
		XIter xIter_short;
		xIter_short.x = (center_vert.x<<8) + 0xFFFF;
		xIter_short.delta_x = (s64(bottom_vert.x - center_vert.x) * lower_rcp)>>8;
		xIter_short.increment((center<<8)-center_vert.y);

		// render
		u8* dst = &target[center*GBA_WIDTH];
		u8* dstStop = &target[bottom*GBA_WIDTH];
		XIter& xIterL = (type==LEFT) ? xIter_short : xIter_long;
		XIter& xIterR = (type==LEFT) ? xIter_long  : xIter_short;
		draw_part(dst, dstStop, uvIter, xIterL, xIterR, duvdx, dudx, dvdx, hLinePtr, mtl);
	}
}

