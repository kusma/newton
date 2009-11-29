#pragma once
#include "support.h"
#include "fixed.h"
#define MAX_GX 30
#define MAX_GY 20

namespace glow {
	void int_vcount();
	u32 setupSprite(int v0, int v1, int v2, int v3);
	// attrib 2 and 1 of sprites
	extern u32 glowSpriteSets[2][21][32] EWRAM_DATA;
		
	void init(const u16 pal[0x20] = NULL);
	void update(const u8* precalc_table, fixed16 time);
	void vblank();
};
