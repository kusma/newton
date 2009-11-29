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


extern const u8 mercury_st[];
extern const u16 mercury_pal[];
extern const u8 plane06completemap_tex[];
extern const u8 env4_tex[];
extern const u8 box10completemap_tex[];
extern const u8 box11completemap_tex[];


namespace {
namespace scene {
#include "../scenes/mercury.h"
};
};

static fixed16 endBeat;
static int blanks = 0;
static void vblank()
{
	fixed16 fader = (endBeat - timer::getBeat())*(1.0/4);
	if (fader<fixed16(1.0)) {
		int amt = fader.get_val()>>8;
		palette::boost(BG_COLORS, mercury_pal, 256, amt-256);
	}
	blanks++;
}


static void init()
{
	VBlankIntrWait();
	int svart=0;
	CpuFastSet(mercury_pal, BG_COLORS, (512 / 4));
	CpuFastSet(&svart, fb::bb, FILL|(240*160/4));
	CpuFastSet(&svart, fb::fb, FILL|(240*160/4));
	fb::setMode(MODE_4 | BG2_ON);
}


namespace parts
{

	void mercury(fixed16 start, fixed16 end)
	{
		endBeat=end;
		part::set_part(init, vblank);
		
		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);
		trede.shadeTable = mercury_st;
		//FORI(256) BG_COLORS[i] = telos_pal[i];
		int swaps = 0;
		blanks = 0;
		
		while (timer::getBeat() < end)
		{
			fixed16 beat = timer::getBeat() - start;
			
			// round the time to avoid interpolation
			int frame = (beat*8).get_val()>>16;
			frame += 256;
			trede.setFrame(frame,0);

			int svart=0;
			if (beat*8<48) CpuFastSet(&svart, fb::bb, FILL|(240*160/4));
			trede.draw(fb::bb);
			
			part::swap();
			swaps++;
		}
	}
};
