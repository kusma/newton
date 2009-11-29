#include "glow.h"
#include "math.h"
#include "exposure.h"
#include "vcount.h"
#include <gba_video.h>
#include <gba_dma.h>
#include <gba_sprites.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
using namespace exposure;
using namespace glow;

// this is timing critical
#define VCOUNT_BIAS 3
	
	






// handtweaka opp mot paletten for a unnga kanter
static u8 rangeluts[8][8] IWRAM_DATA = {
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, // 0
	{ 0, 0, 1, 2, 3, 4, 5, 6 }, // 1
	{ 0, 0, 0, 1, 3, 4, 6, 7 }, // 2
	{ 0, 0, 0, 0, 2, 3, 4, 6 }, // 3
	{ 0, 0, 0, 0, 0, 2, 5, 7 }, // 4
	{ 0, 0, 0, 0, 0, 0, 3, 6 }, // 5
	{ 0, 0, 0, 0, 0, 0, 0, 7 }, // 6
	{ 0, 0, 0, 0, 0, 0, 0, 0, } // 7 - degenerate. force single-color sprite.
};

// creates attr1 and attr2 concatenated to 32 bits
u32 glow::setupSprite(int v0, int v1, int v2, int v3) {
	// find index and s_pal.
	int index;
	int s_pal;
	{
		int index_dark=0;
		int val_dark=v0;
		int index_bright=0;
		int val_bright=v0;
		if (v1<val_dark) {
			val_dark=v1;
			index_dark=1;
		} else if (v1>val_bright) {
			val_bright=v1;
			index_bright=1;
		}
		if (v2<val_dark) {
			val_dark=v2;
			index_dark=2;
		} else if (v2>val_bright) {
			val_bright=v2;
			index_bright=2;
		}
		if (v3<val_dark) {
			val_dark=v3;
			index_dark=3;
		} else if (v3>val_bright) {
			val_bright=v3;
			index_bright=3;
		}

		bool flipped = val_bright < (7-val_dark);

		if (!flipped) {
			index = index_dark;
			s_pal = val_dark;
		} else {
			index = index_bright;
			v0 = 7-v0;
			v1 = 7-v1;
			v2 = 7-v2;
			v3 = 7-v3;
			s_pal = 15-val_bright;
		}
	}


	// find s_spr.
	int s_spr;
	{
		int s1, s2, s3;
		if (index==0) {
			u8* lut = rangeluts[v0];
			s1 = lut[v1];
			s2 = lut[v2];
			s3 = lut[v3];
		} else if (index==1) {
			u8* lut = rangeluts[v1];
			s1 = lut[v0];
			s2 = lut[v3];
			s3 = lut[v2];
		} else if (index==2) {
			u8* lut = rangeluts[v2];
			s1 = lut[v3];
			s2 = lut[v0];
			s3 = lut[v1];
		} else {
			u8* lut = rangeluts[v3];
			s1 = lut[v2];
			s2 = lut[v1];
			s3 = lut[v0];
		}

		s_spr = (s3<<6)|(s2<<3)|(s1);
	}

	// SPRITE
//		s.attr0 = OBJ_Y(y) | OBJ_16_COLOR | OBJ_MODE(1); // mode=semi-trans
//		s.attr1 = OBJ_X(x) | OBJ_SIZE(0) | (index<<12);
//		s.attr2 = OBJ_CHAR(512+s_spr) | OBJ_PALETTE(s_pal);
	{
		u32 attr1 = OBJ_X(0) | OBJ_SIZE(0) | (index<<12);
		u32 attr2 = OBJ_CHAR(512+s_spr) | OBJ_PALETTE(s_pal);
		return (attr2<<16)|(attr1);
	}
}



namespace {
	int vc_seti=0;
	int vc_gy=0;
};


void glow::int_vcount() {
	int& gy=vc_gy;
	OBJATTR* dst = OAM;

//	while (!(REG_DISPSTAT & LCDC_HBL));
	
	for (int lineCount=0; lineCount<4; lineCount++, gy++) {
		int y = gy*8;
		u32* gs = glowSpriteSets[vc_seti][gy];
		int gx=0;
		for (; gx<28; gx+=4, gs+=4, dst+=4) {
			int x = gx*8;
			dst[0].attr0 = OBJ_Y(y) | OBJ_16_COLOR | OBJ_MODE(1);
			dst[0].attr1 = OBJ_X(x) | gs[0];
			dst[0].attr2 = gs[0] >> 16;
			dst[1].attr0 = OBJ_Y(y) | OBJ_16_COLOR | OBJ_MODE(1);
			dst[1].attr1 = OBJ_X(x+8) | gs[1];
			dst[1].attr2 = gs[1] >> 16;
			dst[2].attr0 = OBJ_Y(y) | OBJ_16_COLOR | OBJ_MODE(1);
			dst[2].attr1 = OBJ_X(x+16) | gs[2];
			dst[2].attr2 = gs[2] >> 16;
			dst[3].attr0 = OBJ_Y(y) | OBJ_16_COLOR | OBJ_MODE(1);
			dst[3].attr1 = OBJ_X(x+24) | gs[3];
			dst[3].attr2 = gs[3] >> 16;
		}
		for (; gx<MAX_GX; gx++, gs++, dst++) {
			int x = gx*8;
			dst[0].attr0 = OBJ_Y(y) | OBJ_16_COLOR | OBJ_MODE(1);
			dst[0].attr1 = OBJ_X(x) | gs[0];
			dst[0].attr2 = gs[0] >> 16;
		}
		// end?
		if (gy>=20) return;
	}

	int next_y = gy*8;
	REG_DISPSTAT = VCOUNT(next_y-VCOUNT_BIAS) | (REG_DISPSTAT&0xFF);
}

	
void glow::vblank() {
//	frame_offsetx = global_offsetx + 2;
//	frame_offsety = global_offsety + 2;
//	int old_frame_offsetx = frame_offsetx;
//	int old_frame_offsety = frame_offsety;
//	while (frame_offsetx == old_frame_offsetx) frame_offsetx = global_offsetx + (cheap_rand()&3);
//	while (frame_offsety == old_frame_offsety) frame_offsety = global_offsety + (cheap_rand()&3);
	
	// start the chain of vcount interrupts
	vc_gy=0;
	vc_seti ^= 1;
	glow::int_vcount();
	

	assert(REG_VCOUNT>=160 || REG_VCOUNT<16);
}

INLINE int dither(int v, int sign) {
	// dither
	if (sign==0) {
		v += 0x10;
		if (v>0xFF) v=0xFF;
	}
	return v >> 5;
}


unsigned char sampleGrid[21][32] EWRAM_DATA;
void glow::update(const u8* precalc_table, fixed16 time) {
	int frame = time.get_val()>>16;
	int delta = time.get_val()&0xFFFF;
	const u8* table0 = &precalc_table[31*21*frame];
	const u8* table1 = table0 + 31*21;

	
	

	// lerp frames
	for (int gy=0; gy<=MAX_GY; gy++) {
		for (int gx=0; gx<=MAX_GX; gx++) {
			int v0 = *table0++;
			int v1 = *table1++;
			sampleGrid[gy][gx] = v0 + (((v1-v0)*delta)>>16);
		}
	}

	// create both sets
	for (int seti=0; seti<2; seti++) {
		
		// create spriteset
		for (int gy=0; gy<MAX_GY; gy++) {
			for (int gx=0; gx<MAX_GX; gx++) {
				int v0 = dither(sampleGrid[gy][gx], seti);
				int v1 = dither(sampleGrid[gy][gx+1], seti);
				int v2 = dither(sampleGrid[gy+1][gx], seti);
				int v3 = dither(sampleGrid[gy+1][gx+1], seti);
				glowSpriteSets[seti][gy][gx] = setupSprite(v0, v1, v2, v3);
			}
		}
	}
	
}
