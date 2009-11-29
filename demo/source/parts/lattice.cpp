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
#include "../parts.h"

#include "../fb.h"
#include "../palette.h"
#include "../3dthen.h"
#include "../timer.h"

/* turntable textures */
extern const u16 lattice_pal[];
extern const u8  lattice_tex[];

namespace scene0 {
#define lattice256x32_tex 0 // shutup exporter
#include "../scenes/lattice_buildup.h"
};

namespace scene1 {
#define lattice256x32_tex 0 // shutup exporter
#include "../scenes/lattice.h"
};

static fixed16 endBeat;
static int flash=256;
static int blanks = 0;
static void vblank()
{
	flash-=4;
	if (flash<0) flash=0;

	u16 farg = RGB8(flash, flash, flash);
	palette::add(BG_COLORS, lattice_pal, 256, farg);

	fixed16 fader = (endBeat - timer::getBeat())*(1.0/4);
	if (fader<fixed16(1.0)) {
		int amt = fader.get_val()>>8;
		palette::boost(BG_COLORS, BG_COLORS, 256, amt-256);
	}
	blanks++;
}

static void init()
{
	VBlankIntrWait();
	CpuFastSet(lattice_pal, BG_COLORS, (512 / 4));
	BG_COLORS[0] = 0;
	fb::setMode(MODE_4 | BG2_ON);
	
	REG_BG2PA = 0x100;
	REG_BG2X  = 0;
	REG_BG2PD = 0x100;
	REG_BG2Y  = 0;
}


extern const u16 testmask_mask[];

namespace parts
{

	void lattice(fixed16 start, fixed16 end)
	{
		endBeat=end;
		part::set_part(init, vblank);
		
		Trede trede0(scene0::verts, scene0::vertcount, scene0::faces, scene0::facecount, scene0::uvs, scene0::materials, scene0::framestates, scene0::framecount, 1);
		Trede trede1(scene1::verts, scene1::vertcount, scene1::faces, scene1::facecount, scene1::uvs, scene1::materials, scene1::framestates, scene1::framecount, 1);
		
		int swaps = 0;
		blanks = 0;
		
		int lastBeat=-1;
		while (timer::getBeat() < end)
		{
			fixed16 beat = timer::getBeat() - start;

			int nextBeat = int(beat)>>3;
			bool bling=(nextBeat!=lastBeat);
			lastBeat = nextBeat;

			if (bling) flash=256;

			fixed16 t = beat*8;

			int anim_frame = swaps & 31;
			if (t<fixed16(256) && t>fixed16(256-16)) {
				anim_frame = -anim_frame*2;
				while (anim_frame<0) anim_frame += 32;
			}
			scene0::materials[0].tex[0] = (u8*)lattice_tex + 256 * 64 * anim_frame;
			scene0::materials[0].oneone = true;
			scene1::materials[0].tex[0] = (u8*)lattice_tex + 256 * 64 * anim_frame;
			scene1::materials[0].oneone = true;

			static fixed16 offset=0;
			if (bling) offset = fixed16().make_raw( (cheap_rand()&255)*(1<<16) );
			if (t<256)
			{
				if (t < 64)
				{
					t = t*2;
				}
				else if (t < 128)
				{
					t = fixed16(256.5)-t*2;
					if (t<0) t=0;
				}
				else
				{
					t-=8;
					if (t<129) t=129;
				} 
				trede0.setTime( t );
				trede0.draw(fb::bb);
			} else {
				trede1.setTime(t + 100 + offset);
				trede1.draw(fb::bb);
			}

			part::swap();
			swaps++;
		}

	}
};
