#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <stdio.h>
#include "../fb.h"
#include "../grid.h"
#include "../grideffects.h"
#include "../palette.h"
#include "../part.h"

#include "../timer.h"
#include "../beatcounter.h"

extern const u16 part_growy_pal[];
extern const u8 flextextiles_tex[];

static volatile int blanks = 0;
static volatile int flash  = 256;
static void vblank()
{
	blanks++;
	fixed16 time = timer::getBeat();
	palette::boost(BG_COLORS, part_growy_pal, 256, 128 + (sin(time).get_val() >> 9));
	flash -= 2;
	if (0 == flash) flash = 256;

#if 0
	REG_DMA0CNT = 0;
	DMA0COPY(&currents[0], &REG_BG2X, DMA_HBLANK | DMA_REPEAT | DMA_DST_FIXED | DMA_SRC_INC | DMA32 | 1);

	s32 *dst = currents;
	for (int i = GBA_HEIGHT; i; --i)
	{
		int rands = 0;
		int offs = fixed_fsin(i + blanks) >> 5;
		offs += fixed_fsin((i - blanks) * 5) >> 6;
		rands = ((s32(cheap_rand()) >> 22) * offs >> 9) ;
		*dst++ = rands; // *targ;
	}
#endif
}

static void init()
{
	VBlankIntrWait();
	SetMode(MODE_4 | BG2_ON);
	REG_BG2X = 0;
	CpuFastSet(part_growy_pal, BG_COLORS, 512 / 4);
	BG_COLORS[0] = 0;
}

grid g EWRAM_DATA;

const fixed16 bc_data[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8
};
static BeatCounter bc = BEATCOUNTER(bc_data);


namespace parts
{
	void grid(fixed16 start, fixed16 end)
	{
		part::set_part(init, vblank);
		
		int swaps = 0;
		blanks = 0;
		while (timer::getBeat() < end)
		{
			fixed16 time = timer::getBeat() - start;
			time += bc.getPos(time);
			time *= fixed16(0.01);
			
			grideffects::flat(g, (time + cos(time)).get_val(), sin(time * fixed16(0.95)).get_val());
			
			draw_grid(fb::bb, g, flextextiles_tex);
			part::swap();
			swaps++;
		}
	}
};
