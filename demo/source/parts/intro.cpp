#include "flikk.h"
#include "math.h"
#include "exposure.h"
#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_sprites.h>
#include "fb.h"
#include "config.h"
#include "palette.h"
#include <pimpmobile.h>

extern const u16 starttxt_flikk[];


static volatile bool regen = true;
static volatile bool nextframe = false;
static int frameno = -1;
static bool goback = false;
static int flash = 0;
static int blanks = 0;

static s32 targets[GBA_HEIGHT] EWRAM_DATA;
static s32 currents[GBA_HEIGHT] EWRAM_DATA;

static void vblank()
{
	pimp_vblank();
	
	s32 *dst  = currents;
	s32 *targ = targets;
	s32 offs = fixed_sin(blanks - 100) >>5;
	s32 rands = s32(cheap_rand()) >> 18;
	for (int i = GBA_HEIGHT; i; --i)
	{
		rands += (s32(cheap_rand()) >> 22);
		*dst++ = *targ + ((rands * *targ) >> 14); // *targ;
		targ++;
	}
	
//	REG_BG2X = blanks * 256;
	REG_DMA0CNT = 0;
	DMA0COPY(&currents[0], &REG_BG2X, DMA_HBLANK | DMA_REPEAT | DMA_DST_FIXED | DMA_SRC_INC | DMA32 | 1);

	if (regen)
	{
		s32 *dst = targets;
		s32 offs = s32(cheap_rand()) >> 22;
		rands = 0;
		for (int i = GBA_HEIGHT; i; --i)
		{
			offs = fixed_fsin(i + blanks) >> 4;
			offs += fixed_fsin((i - blanks) * 5) >> 5;
			rands += ((s32(cheap_rand()) >> 22) * offs >> 11) ;
			*dst++ = (offs + rands); // *targ;
		}
		regen = false;
	}

//	REG_BG2X = pimp_get_row() * 256;
//	DMA_HBLANK

	blanks++;
	pimp_frame();
}

static void callback(int type, int data)
{
	if (data==0) {
		regen=true;
		flash = max(flash, 0x40);
	}
	if (data==1) {
		nextframe=true;
		flash = max(flash, 0x80);
	}
	if (data==2) {
		goback = !goback;
	}
}



namespace parts
{
	void intro(int len, int startframe)
	{
		pimp_set_callback(callback);
		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);

		Flikk flikk(starttxt_flikk);


		regen = true;
		nextframe = false;
		frameno = -1;
		goback = false;
		flash = 0;
		blanks = 0;

		


		int swaps = 0;
		blanks = 0;

		int start = pimp_get_order();

		while (pimp_get_order() >= start && pimp_get_order() < start+len)
		{
				
			if (nextframe)
			{
				frameno++;
				nextframe=false;
			}

			if (frameno >= 0) {
				flikk.setFrame(frameno+startframe);
				flikk.drawMov(fb::bb, 0);
			} else {
				int svart=0;
				CpuFastSet(&svart, fb::bb, FILL|COPY32|(240*160/4));
			}
			
			if (goback && frameno > -1 && (swaps&1)==0) frameno--;
			
			VBlankIntrWait();
			fb::swap();

			FORI(16) {
				int val = ((i - 1) * (0xFFFF / 15)) >> 8;
				if (i==0) val=0;
				val += flash;
				if (val > 255) val = 255;
				BG_COLORS[i] = RGB8(val, val, val);
			}
			if (flash > 0) flash -= 8;
			
			// decrease noise
			FORI(GBA_HEIGHT) {
				int old = targets[i];
				int nju = (targets[i] * 220) >> 8;
				if (old==nju) nju=0;
				targets[i] = nju;
			}

			swaps++;
		}

		pimp_set_callback(NULL);
		REG_BG2X  = 0;
		REG_BG2Y  = 0;
		REG_BG2PA = 0x100;
		REG_BG2PD = 0x100;
	}
};
