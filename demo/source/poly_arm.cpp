#include <gba_video.h>
#include <gba_dma.h>
#include <mappy.h>
#include "poly.h"
#include "support.h"
#include "math.h"
#include "exstructs.h"
#include <gba_systemcalls.h>

#define SHIFT 16
#define LEFT  1
#define RIGHT 2

#define FILLER_SHIFT256_1X1
#define FILLER_TEX256_1X1
#define FILLER_TEX256_2X1
//#define FILLER_NMAP_1X1
//#define FILLER_NMAP_2X1
#define FILLER_EBUMP_1X1
//#define FILLER_EBUMP_2X1


namespace {

	// stuff used by draw_part
	const s16* global_light;

	void hline_shift256_1x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs) IWRAM_CODE;
	void hline_tex256_2x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs) IWRAM_CODE;
	void hline_nmap_2x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs) IWRAM_CODE;
	void hline_ebump_2x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs) IWRAM_CODE;
	void hline_tex256_1x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs) IWRAM_CODE;
	void hline_nmap_1x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs) IWRAM_CODE;
	void hline_ebump_1x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs) IWRAM_CODE;


	void hline_shift256_1x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs) {
#ifndef FILLER_SHIFT256_1X1
		assert(0);
#else
		const u8* tex = tex0;
		if (x2>GBA_WIDTH) x2=GBA_WIDTH; 
		if (x1<0) {
			uv -= ((duvdx&0xFFFF0000)*x1)|u16(duvdx*x1);
			x1=0;
		}
		if (x2 <= x1) return;
		
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
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			u8 col = tex[index];
			val = (val&0x00FF) | (((val & 0xFF00) >> col) & 0xFF00);
			x1++;
			uv += duvdx;
		}


		dst += x1;
		s32 counter = (x2-x1)>>1;
		register unsigned c = counter >> 4;
		switch (counter&15) {
			do {
			ITERATION;
			case 15: ITERATION;
			case 14: ITERATION;
			case 13: ITERATION;
			case 12: ITERATION;
			case 11: ITERATION;
			case 10: ITERATION;
			case 9:  ITERATION;
			case 8:  ITERATION;
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
			u8 col = tex[index];
			val = (val&0xFF00) | ((val & 0xFF) >> col);
			x1++;
		}
#endif
	}


	void hline_tex256_2x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex, const u8* tex1, int palOffs) {
#ifndef FILLER_TEX256_2X1
		assert(0);
#else
		if (x2>GBA_WIDTH) x2=GBA_WIDTH; 
		if (x1<0) {
			uv -= ((duvdx&0xFFFF0000)*x1)|u16(duvdx*x1);
			x1=0;
		}
		if (x2 <= x1) return;
#define ITERATION {\
	u32 index = (uv>>24)|(uv&0xFF00);\
	u8 col = tex[index];\
	*dst = col;\
	dst += 2;\
	uv += (duvdx<<1); \
}

		if (x1&1) {
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			u8 col = tex[index];
			val = (val&0x00FF)|(u32(col)<<8);
			x1++;
			uv += duvdx;
		}

		dst += x1;
		s32 counter = (x2-x1)>>1;
		register unsigned c = counter >> 4;
		switch (counter&15) {
			do {
			ITERATION;
			case 15: ITERATION;
			case 14: ITERATION;
			case 13: ITERATION;
			case 12: ITERATION;
			case 11: ITERATION;
			case 10: ITERATION;
			case 9:  ITERATION;
			case 8:  ITERATION;
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
			u8 col = tex[index];
			val = (val&0xFF00)|col;
			x1++;
		}
#endif
	}


	void hline_tex256_1x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs) {
#ifndef FILLER_TEX256_1X1
		assert(0);
#else
		const u8* tex = tex0;
		if (x2>GBA_WIDTH) x2=GBA_WIDTH; 
		if (x1<0) {
			uv -= ((duvdx&0xFFFF0000)*x1)|u16(duvdx*x1);
			x1=0;
		}
		if (x2 <= x1) return;
		
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
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			u8 col = tex[index];
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
			u8 col = tex[index];
			val = (val&0xFF00)|col;
			x1++;
		}
#endif
	}



	void hline_nmap_2x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs) {
#ifndef FILLER_NMAP_2X1
		assert(0);
#else
		const s16* tex = (s16*)tex0;
		s32 lightx = global_light[0];
		s32 lighty = global_light[1];
		s32 lightz = global_light[2];
		if (x2>GBA_WIDTH) x2=GBA_WIDTH; 
		if (x1<0) {
			uv -= ((duvdx&0xFFFF0000)*x1)|u16(duvdx*x1);
			x1=0;
		}
		if (x2 <= x1) return;


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
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			s32 col = tex[index];
			STUFF(col, col);
			val = (val&0x00FF)|(u32(col)<<8);
			x1++;
			uv += duvdx;
		}

		dst += x1;
		s32 counter = (x2-x1)>>1;
		unsigned c = counter >> 4;
		{
			switch (counter&15) {
				do {
				ITERATION;
				case 15: ITERATION; 
				case 14: ITERATION;
				case 13: ITERATION;
				case 12: ITERATION;
				case 11: ITERATION;
				case 10: ITERATION;
				case 9:  ITERATION;
				case 8:  ITERATION;
				case 7:  ITERATION;
				case 6:  ITERATION_ASM;
				case 5:  ITERATION;
				case 4:  ITERATION;
				case 3:  ITERATION;
				case 2:  ITERATION;
				case 1:  ITERATION;
				case 0:;  
				} while (c--);
			}
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



	void hline_nmap_1x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0_8, const u8* tex1, int palOffs) {
#ifndef FILLER_NMAP_1X1
		assert(0);
#else
		const s16* tex = (s16*)tex0_8;
		s32 lightx = global_light[0];
		s32 lighty = global_light[1];
		s32 lightz = global_light[2];
		if (x2>GBA_WIDTH) x2=GBA_WIDTH; 
		if (x1<0) {
			uv -= ((duvdx&0xFFFF0000)*x1)|u16(duvdx*x1);
			x1=0;
		}
		if (x2 <= x1) return;


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
			u32 index = (uv>>24)|(uv&0xFF00);
			u16& val = ((u16*)dst)[x1>>1];
			s32 col = tex[index];
			STUFF(col, col);
			val = (val&0x00FF)|(u32(col)<<8);
			x1++;
			uv += duvdx;
		}

		dst += x1;
		s32 counter = (x2-x1)>>1;
		unsigned c = counter >> 4;
		{
			switch (counter&15) {
				do {
				ITERATION;
				case 15: ITERATION; 
				case 14: ITERATION;
				case 13: ITERATION;
				case 12: ITERATION;
				case 11: ITERATION;
				case 10: ITERATION;
				case 9:  ITERATION;
				case 8:  ITERATION;
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



	void hline_ebump_2x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0_8, const u8* tex1, int palOffs) {
#ifndef FILLER_EBUMP_2X1
		assert(0);
#else
		const s16* tex = (s16*)tex0_8;
		s32 lightangle = -global_light[0];
		
		if (x2>GBA_WIDTH) x2=GBA_WIDTH; 
		if (x1<0) {
			uv -= ((duvdx&0xFFFF0000)*x1)|u16(duvdx*x1);
			x1=0;
		}
		if (x2 <= x1) return;



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
		register unsigned c = counter >> 4;
		switch (counter&15) {
			do {
			ITERATION;
			case 15: ITERATION;
			case 14: ITERATION;
			case 13: ITERATION;
			case 12: ITERATION;
			case 11: ITERATION;
			case 10: ITERATION;
			case 9:  ITERATION;
			case 8:  ITERATION;
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



	void hline_ebump_1x1(u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs) {
#ifndef FILLER_EBUMP_1X1
		assert(0);
#else
		const s16* tex = (s16*)tex0;
		s32 lightangle = -global_light[0];
		
		if (x2>GBA_WIDTH) x2=GBA_WIDTH; 
		if (x1<0) {
			uv -= ((duvdx&0xFFFF0000)*x1)|u16(duvdx*x1);
			x1=0;
		}
		if (x2 <= x1) return;



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
		register unsigned c = counter >> 4;
		switch (counter&15) {
			do {
			ITERATION;
			case 15: ITERATION;
			case 14: ITERATION;
			case 13: ITERATION;
			case 12: ITERATION;
			case 11: ITERATION;
			case 10: ITERATION;
			case 9:  ITERATION;
			case 8:  ITERATION;
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




	struct XIter {
		s32 x;
		s32 delta_x;
		INLINE void increment() {
			x+=delta_x;
		}
		INLINE void increment(int count) {
			x+=delta_x*count;
		}
	};

#define PACKED_INTERPOLATION

	struct IterL {
	private:
#ifdef PACKED_INTERPOLATION	
		u32 uv;
		u32 delta_uv;
#else
		u32 u, v;
		u32 delta_u;
		u32 delta_v;
#endif
	public:
		INLINE void increment()
		{
#ifdef PACKED_INTERPOLATION
			uv +=delta_uv;
#else
			u += delta_u;
			v += delta_v;
#endif
		}
		
		INLINE void increment(int count)
		{
#ifdef PACKED_INTERPOLATION
			uv += ((delta_uv&0xFFFF0000)*count)|u16(delta_uv*count);
#else
			u += delta_u * count;
			v += delta_v * count;
#endif
		}
		
		INLINE s32 setUv(s32 u, s32 v)
		{
#ifdef PACKED_INTERPOLATION
			this->uv = (u32(u) & 0xFFFF0000) | ((u32(v) >> 16) & 0xFFFF);
#else
			this->u = u;
			this->v = v;
#endif
		}
		
		INLINE s32 setUvDelta(s32 u_delta, s32 v_delta)
		{
#ifdef PACKED_INTERPOLATION
			this->delta_uv = (u32(u_delta) & 0xFFFF0000) | ((u32(v_delta) >> 16) & 0xFFFF);
#else
			this->delta_u = u_delta;
			this->delta_v = v_delta;
#endif
		}
		
		INLINE s32 getUv()
		{
#ifdef PACKED_INTERPOLATION
			return uv;
#else
			return (u & 0xFFFF0000) | ((v >> 16) & 0xFFFF);
#endif
		}
	};




	typedef void (*HLinePtr) (u8* dst, int x1, int x2, u32 uv, s32 duvdx, const u8* tex0, const u8* tex1, int palOffs);


	INLINE void draw_part(u8* dst, IterL& iterL, XIter& xiterL, XIter& xiterR, int count, s32 duvdx, HLinePtr hLinePtr, const Material& mtl) {
		for (; count; count--) {
			int xl = xiterL.x>>16;
			int xr = xiterR.x>>16;
			hLinePtr(dst, xl, xr, iterL.getUv(), duvdx, mtl.tex[0], mtl.tex[1], mtl.palOffs);
//			hLinePtr(dst, xl, xr, iterL.uv, duvdx, mtl.tex[0], mtl.tex[1], mtl.palOffs);
			iterL.increment();
			xiterL.increment();
			xiterR.increment();
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

	unsigned int index = 0;
	if ((v[0].y >> 8) < (v[1].y >> 8)) index |= (1 << 2) * 8;
	if ((v[1].y >> 8) < (v[2].y >> 8)) index |= (1 << 1) * 8;
	if ((v[2].y >> 8) < (v[0].y >> 8)) index |= (1 << 0) * 8;

	int type				= conf_tab[index++];
	const PolyVertex& top_vert	= v[conf_tab[index++]];
	const PolyVertex& center_vert	= v[conf_tab[index++]];
	const PolyVertex& bottom_vert	= v[conf_tab[index++]];
	const PolyVertex& right_vert	= v[conf_tab[index++]];
	const PolyVertex& left_vert	= v[conf_tab[index++]];

	int top		= top_vert.y >> 8;
	int center	= center_vert.y >> 8;
	int bottom	= bottom_vert.y >> 8;
	if (top>=clip_bottom || bottom<=clip_top) return;

	s32 duvdx;
//	s32 duvdx_half;
	{
		s32 dX10 = (v[1].x >> 8) -(v[0].x >> 8);
		s32 dX21 = (v[2].x >> 8) -(v[1].x >> 8);
		s32 dY10 = (v[1].y >> 8) -(v[0].y >> 8);
		s32 dY21 = (v[2].y >> 8) -(v[1].y >> 8);

		s32 area = dX21*dY10 - dX10*dY21;
		if (area<1) return;
		
//		s32 area_inv =s32((1 << 16) / (float(area) / 256));
		s32 area_inv = rcp(area<<8);

		s32 du10 = v[1].u - v[0].u;
		s32 du21 = v[2].u - v[1].u;
		s32 dv10 = v[1].v - v[0].v;
		s32 dv21 = v[2].v - v[1].v;
#if 1
		s32 dudx = (s64(du21*dY10 - du10*dY21) * area_inv) >> 8;
		s32 dvdx = (s64(dv21*dY10 - dv10*dY21) * area_inv) >> 8;
#else
		s32 dudx = int((float(du21*dY10 - du10*dY21) / area) * 65536);
		s32 dvdx = int((float(dv21*dY10 - dv10*dY21) / area) * 65536);
#endif

		duvdx = (dudx & 0xFFFF0000) | (u32(dvdx)>>16); 
//		duvdx_half = ((dudx>>1)&0xFFFF0000) | (u32(dvdx>>1)>>16);
	}

	s32 count;
	u8* dst;

	s32 long_rcp  = ircp((bottom_vert.y >> 8) - (top_vert.y >> 8));
	s32 left_rcp  = ircp((left_vert.y   >> 8) - (top_vert.y >> 8));

	XIter xiter_long;
	xiter_long.x = (top_vert.x>>8)<<SHIFT;
	xiter_long.delta_x = ((bottom_vert.x >> 8) - (top_vert.x >> 8)) * long_rcp;

	IterL iterL;

	// finn uv'er
	{
		iterL.setUv(top_vert.u << 16, top_vert.v << 16);
		s32 delta_u = (s64(left_vert.u - top_vert.u) * left_rcp);
		s32 delta_v = (s64(left_vert.v - top_vert.v) * left_rcp);
		iterL.setUvDelta(delta_u, delta_v);
	}

	// -y clipping
	if ((top_vert.y >> 8)<clip_top) {
		xiter_long.increment(clip_top-(top_vert.y>>8));
		iterL.increment(clip_top-(top_vert.y>>8));
	}

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


	// UPPER
	if (center>clip_top) { // upper is visible
		if (center>clip_bottom) center = clip_bottom; // +y clipping

		s32 short_rcp = ircp((center_vert.y >> 8) - (top_vert.y >> 8));

		XIter xiter_short;
		xiter_short.x = (top_vert.x>>8)<<SHIFT;
		xiter_short.delta_x = ((center_vert.x >> 8) - (top_vert.x >> 8)) * short_rcp;

		if ((top_vert.y>>8)<clip_top) { // -y clipping
			top=clip_top; // -y stuff
			xiter_short.increment(clip_top-(top_vert.y >> 8));
		}
		dst = &target[top*GBA_WIDTH];
		count = center-top;

		XIter& xiterL = (type==RIGHT) ? xiter_long  : xiter_short;
		XIter& xiterR = (type==RIGHT) ? xiter_short : xiter_long;
		draw_part(dst, iterL, xiterL, xiterR, count, duvdx, hLinePtr, mtl);
	}

	// LOWER
	if (center<clip_bottom) { // lower is visible
		if (bottom>clip_bottom) bottom = clip_bottom; // +y clipping

		s32 short_rcp = ircp((bottom_vert.y >> 8) - (center_vert.y >> 8));

		XIter xiter_short;
		xiter_short.x = (center_vert.x>>8)<<SHIFT;
		xiter_short.delta_x = ((bottom_vert.x >> 8) - (center_vert.x >> 8)) * short_rcp;

		// oppdater uv for lower
		if (type==LEFT) {
		
			iterL.setUv(center_vert.u << 16, center_vert.v << 16);
			s32 delta_u = (s64(bottom_vert.u - center_vert.u) * short_rcp);
			s32 delta_v = (s64(bottom_vert.v - center_vert.v) * short_rcp);
			iterL.setUvDelta(delta_u, delta_v);
			
			if ((center_vert.y >> 8)<clip_top) iterL.increment(clip_top-(center_vert.y >> 8));
		}

		if ((center_vert.y>>8)<clip_top) { // -y clipping
			xiter_short.increment(clip_top-(center_vert.y>>8));
			dst = &target[clip_top*GBA_WIDTH];
			count = bottom-clip_top;
		} else {  // no -y clipping
			dst = &target[center*GBA_WIDTH];
			count = bottom-center;
		}

		XIter& xiterL = (type==RIGHT) ? xiter_long  : xiter_short;
		XIter& xiterR = (type==RIGHT) ? xiter_short : xiter_long;
		draw_part(dst, iterL, xiterL, xiterR, count, duvdx, hLinePtr, mtl);
	}
}

