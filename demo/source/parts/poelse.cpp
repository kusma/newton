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

extern u8  poelse_st[];
extern u16 poelse_pal[];
extern u8  penv_tex[];
extern u8  roomcompletemap_tex[];

namespace scene {
#include "../scenes/poelse.h"
};

static int flash=256;
static int blanks = 0;
static void vblank()
{
	flash-=2;
	if (flash<0) flash=0;
	u16 farg = RGB8(flash, flash, flash);
	palette::add(BG_COLORS, poelse_pal, 256, farg);
	noise::update();

	blanks++;
}


static void init()
{
	VBlankIntrWait();
	CpuFastSet(poelse_pal, BG_COLORS, (512 / 4));
	fb::setMode(MODE_4 | BG2_ON | OBJ_ON);
	
	noise::setup();
	REG_BLDCNT = BIT(10) | BIT(6) | BIT(4) | BIT(13);
	REG_BLDALPHA = (3) | (16<<8);
}


extern const u16 testmask_mask[];

namespace parts
{

	void poelse(fixed16 start, fixed16 end)
	{
		part::set_part(init, vblank);
		
		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);
		trede.shadeTable = poelse_st;
		
		int swaps = 0;
		blanks = 0;
		
		int offset=0;
		int lastBeat=-1;
		while (timer::getBeat() < end)
		{
			fixed16 beat = timer::getBeat() - start;
//			FORI(240*160/2) fb::bb[i*2]=0;


			int nextBeat = int(beat)>>3;
			bool bling=(nextBeat!=lastBeat);
			lastBeat = nextBeat;

			if (bling) flash=64;


			trede.setTime(beat*8 + offset);
			if (beat*8>fixed16(256)) {
				int tulle = cheap_rand()&63;
				if (tulle==0) {
					offset += 256;
				} else if (tulle==1) {
					offset += 128;
				}
			};
			trede.draw(fb::bb);
//			decomp::rleImage4shift(fb::bb, fb::bb+GBA_HEIGHT*GBA_WIDTH, testmask_mask, 0);
			
			part::swap();
			swaps++;
		}
	}
};
