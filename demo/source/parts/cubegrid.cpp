#include "../exstructs.h"

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
#include "../part.h"
#include "../fb.h"
#include "../parts.h"
#include "../palette.h"
#include "../3dthen.h"
#include "../noise.h"
#include "../decomp.h"
#include "../timer.h"

/* turntable textures */
extern const u8 cubegrid_st[];
extern const u16 cubegrid_pal[];
extern const u8 cubegrid_tex[];
namespace{
namespace scene {
#include "../scenes/scene_cubegrid2.h"
};
};

static s32 targets[GBA_HEIGHT] EWRAM_DATA;
static s32 currents[GBA_HEIGHT] EWRAM_DATA;
static int deltas[GBA_HEIGHT] EWRAM_DATA;
static int blanks = 0;
static bool regen = true;

static void vblank()
{
	blanks++;
	
	noise::update();
	fixed16 a = fixed16(5) + (sin(fixed16(blanks) >> 8)) * 5;
	int alpha = 5 + (a.get_val() >> 16);
	
	alpha += cheap_rand() & 3;
	if (alpha > 16) alpha = 16;
	if (alpha < 0) alpha = 0;
	//REG_BLDALPHA = (alpha) | ((16 - alpha)<<8);

	// powered alpha
	int alphaSqr = (alpha*alpha)>>4;
	REG_BLDALPHA = (alphaSqr) | ((16 - alphaSqr)<<8);

	s32 *dst  = currents;
	s32 *targ = targets;
	s32 offs = fixed_sin(blanks - 100) >>5;
	s32 rands = s32(cheap_rand()) >> 18;
	for (int i = GBA_HEIGHT; i; --i)
	{
//		rands += (s32(cheap_rand()) >> 22);
		rands = a.get_val() >> 10;
		*dst++ = (*targ * rands) >> 14; // *targ;
		targ++;
	}
	
//	REG_BG2X = blanks * 256;
	REG_DMA0CNT = 0;
	DMA0COPY(&currents[0], &REG_BG2X, DMA_HBLANK | DMA_REPEAT | DMA_DST_FIXED | DMA_SRC_INC | DMA32 | 1);
	
	if (0 == (blanks % 4)) regen = true;
	
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
			
			int temp = (offs + rands);
			temp = (temp * temp) >> 10;
			temp += rands;
//			temp = (temp * temp) >> 12;
			*dst++ = -(1 << 11) + temp; // *targ;
		}
		regen = false;
	}
}


static void init()
{
	VBlankIntrWait();
	CpuFastSet(cubegrid_pal, BG_COLORS, (512 / 4));
	BG_COLORS[0] = 0;
	SetMode(MODE_4 | BG2_ON | OBJ_ON | OBJ_1D_MAP);
	noise::setup();
	REG_BLDCNT = BIT(10) | BIT(6) | BIT(4) | BIT(13);
	REG_BLDALPHA = (16) | (16<<8);
}


extern const u16 testmask_mask[];

namespace parts
{

	void cubegrid(fixed16 start, fixed16 end)
	{
		part::set_part(init, vblank);
		
		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);
		trede.shadeTable=cubegrid_st;
		int swaps = 0;
		blanks = 0;
		
		while (timer::getBeat() < end)
		{
			fixed16 beat = timer::getBeat() - start;
//			trede.setFrame(timer::getBeat() - start, 0);
//			trede.setFrame(beat * 16, 0);
			trede.setFrame(beat * 24, 0);
			trede.draw(fb::bb);
//			decomp::rleImage4shift(fb::bb, fb::bb+GBA_HEIGHT*GBA_WIDTH, testmask_mask, 0);
			
			part::swap();
			swaps++;
		}
		REG_DMA0CNT = 0;
		REG_BG2X = 0;
	}
};
