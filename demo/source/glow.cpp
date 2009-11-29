#include "math.h"
#include "exposure.h"
#include "glow.h"
#include <gba_video.h>
#include <gba_dma.h>
#include <gba_sprites.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>


u32 glow::glowSpriteSets[2][21][32] EWRAM_DATA;



extern "C"
{
	extern const u8  glow_pali[];
	extern const u8  glow_spr[];
}

static const u16 glowgrad[0x20] = {
	0x0000, 0x0000, 0x0421, 0x0842, 0x0c63, 0x1084, 0x14a5, 0x18c5, 
	0x1ce6, 0x2105,	0x2525, 0x2546, 0x2966, 0x2d66, 0x3186, 0x35a7, 
	0x39c7, 0x3de8, 0x4208, 0x4629,	0x4a4a, 0x4e6b, 0x528c, 0x56ad,
	0x5eef, 0x6311, 0x6b53, 0x7395, 0x77b7, 0x7bd9,	0x7ffa, 0x7ffF,
};




void glow::init(const u16 pal[0x20]) {
	// upload sprites
	CpuFastSet(glow_spr, BITMAP_OBJ_BASE_ADR, 512*8*8/2/4);
	
	// generate palette
	if (!pal) pal=glowgrad;
	FORI(256) {
		int index = glow_pali[i];
		OBJ_COLORS[i] = pal[index];	
	}
	
	// hw state
//	REG_BLDCNT = BIT(10) | (1<<6) | BIT(4);
	REG_BLDCNT = (BIT(11) | BIT(10) | BIT(9) | BIT(8)) | (1<<6) | BIT(4);
//	REG_BLDALPHA = (16) | (16<<8);
	
	irqSet(IRQ_VCOUNT, glow::int_vcount);
	irqEnable(IRQ_VCOUNT);
}


