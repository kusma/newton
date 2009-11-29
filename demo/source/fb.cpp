#include "fb.h"	
#include <gba_sprites.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_dma.h>

namespace fb {
	u16 temppal[256] EWRAM_DATA;
	u16 tempxpal[256*4] EWRAM_DATA;
	u8* bb = (u8*)*MODE5_BB;
	u8* fb = (u8*)*MODE5_FB;
	
	void reset_buffers()
	{
		if (REG_DISPCNT&BACKBUFFER)
		{
			bb= (u8*)*MODE5_FB;
			fb= (u8*)*MODE5_BB;
		} else {
			bb= (u8*)*MODE5_BB;
			fb= (u8*)*MODE5_FB;
		}
	}

	void reset() {
		irqDisable(IRQ_HBLANK);
		irqDisable(IRQ_VCOUNT);
		REG_DMA0CNT = 0;

		int svart=0;
		CpuFastSet(&svart, BG_COLORS, FILL | (512/4));
		CpuFastSet(&svart, OBJ_COLORS, FILL | (512/4));
		CpuFastSet(&svart, fb, FILL | (240*160/4));
		CpuFastSet(&svart, bb, FILL | (240*160/4));
		bb = (u8*)*MODE5_BB;
		fb = (u8*)*MODE5_FB;
		
		reset_buffers();
		REG_DISPCNT &= ~BACKBUFFER;
		
		for (unsigned i = 0; i < 128; i++) OAM[i].attr0 = OBJ_DISABLE;
		REG_BLDCNT = BIT(4) | (1<<6)| BIT(10); 
		REG_BLDALPHA = (16) | (16<<8);
		//SetInterrupt(Int_Hblank, hblank_empty);
		//DisableInterrupt(Int_Hblank);
		REG_BG2X=0;
		REG_BG2Y=0;
		SetMode((LCDC_BITS)(MODE_4 | BG2_ENABLE | OBJ_ENABLE | OBJ_1D_MAP));
	}
	
	void swap() {
		REG_DISPCNT ^= BACKBUFFER;
		reset_buffers();
	}
	
	void waitline(int x) {
		while (REG_VCOUNT>x);
		while (REG_VCOUNT<x);
	}
	void overlay(const void* oam, const void* spr, const u16* pal) {
		if (pal) CpuFastSet(pal, OBJ_COLORS, 512/4);
		CpuSet(oam,  OAM, (75 * sizeof(OAM[0])) / 2);
		CpuFastSet(spr, BITMAP_OBJ_BASE_ADR, 8 * 8 * 128 / 2);
	}


	u16 ewram_pal[256] EWRAM_DATA;
	u8  ewram_fb[GBA_WIDTH * GBA_HEIGHT] EWRAM_DATA;

	inline void wait_vblank()
	{
		while (REG_VCOUNT > GBA_HEIGHT);
		while (REG_VCOUNT < GBA_HEIGHT);
	}

	void hyperblink(const u8 *picture_raw, const u16 *picture_pal)
	{
		int reg_ie = REG_IE;
		REG_IE = 0;

		u32 white = ~0;
		u32 black = 0;

		CpuFastSet(BG_COLORS, ewram_pal, COPY32 | (512 / 4));

		wait_vblank();
		CpuFastSet(&black, BG_COLORS, FILL | COPY32 | (512 / 4));
		CpuFastSet(fb::fb, ewram_fb, COPY32 | ((GBA_WIDTH * GBA_HEIGHT) / 4));

		wait_vblank();

		wait_vblank();
		CpuFastSet(&white, BG_COLORS, FILL | COPY32 | (512 / 4));
		CpuFastSet(picture_raw, fb::fb, COPY32 | ((GBA_WIDTH * GBA_HEIGHT) / 4));

		wait_vblank();

		wait_vblank();
		CpuFastSet(picture_pal, BG_COLORS, COPY32 | (512 / 4));

		wait_vblank();

		wait_vblank();
		CpuFastSet(ewram_pal, BG_COLORS, COPY32 | (512 / 4));
		CpuFastSet(ewram_fb,  fb::fb, COPY32 | ((GBA_WIDTH * GBA_HEIGHT) / 4));

		REG_IE = reg_ie;
	}

};


