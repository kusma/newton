#pragma once

#include "base.h"
#include <gba_video.h>

namespace fb {
	extern u8* fb;
	extern u8* bb;
	void reset();
	void swap();
	void hyperblink(const u8 *picture_raw, const u16 *picture_pal);

	void reset_buffers();
	
	INLINE void setMode(LCDC_BITS mode)
	{
		SetMode(mode | (REG_DISPCNT & BACKBUFFER)); /* preserve backbuffer-flag */
	}
	
	INLINE void setMode(int mode)
	{
		setMode((LCDC_BITS)mode);
	}


	void waitline(int y);
	INLINE void err() {
		bb[0] = 5;
	}
	void overlay(const void* oam, const void* spr, const u16* pal=NULL);
	
	
	INLINE void plot_pixel(u8 *bb, int x, int y, int color)
	{
		if (x < 0) return;
		if (x > GBA_WIDTH) return;
		if (y < 0) return;
		if (y > GBA_HEIGHT) return;
		
		bb[y * GBA_WIDTH + x] = color;
	}

	extern u16 temppal[256] EWRAM_DATA;
	extern u16 tempxpal[256*4] EWRAM_DATA;
};

#define DECLARE_OVERLAY(name) \
extern const u8  name##_tileset_raw[]; \
extern const u16 name##_tileset_pal[]; \
extern const u16 name##_oam[];

#define OVERLAY(name) fb::overlay(name##_oam, name##_tileset_raw, name##_tileset_pal)
