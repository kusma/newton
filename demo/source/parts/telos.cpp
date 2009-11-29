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
#include "../glow.h"

extern const u8 telos_glow[];
extern const u8 ref6_tex[];
extern const u8 ned_tex[];
extern const u8 opp_tex[];
extern const u8 t01_tex[];
extern const u16 telos_pal[];
extern const u8 telos_st[];
extern const u8 t02_tex[];
extern const u8 t03_tex[];
extern const u8 t04_tex[];

namespace {
namespace scene {
#include "../scenes/telos.h"
};
};

int flash = 32;
static void vblank()
{
	glow::vblank();
	int f = RGB5(flash, flash, flash);
	palette::add(BG_COLORS, telos_pal, 256, f);
	flash -= 1;
	if (flash < 0) flash = 0;
}


static void init()
{
	VBlankIntrWait();

	int white = (RGB5(31,31,31) << 8) | RGB5(31,31,31);
	palette::add(BG_COLORS, telos_pal, 256, RGB5(31,31,31));
	glow::init();
//	CpuFastSet(&white, BG_COLORS, FILL|(512 / 4));
	
	SetMode(MODE_4 | BG2_ON | OBJ_ON);

	REG_BG2PA = 0x100;
	REG_BG2X  = 0;
	REG_BG2PD = 0x100;
	REG_BG2Y  = 0;

}

namespace parts
{

	void telos(fixed16 start, fixed16 end)
	{
		part::set_part(init, vblank);
		
		VBlankIntrWait();
		BG_COLORS[0] = RGB5(31, 31, 31);
		SetMode(MODE_4);
		fb::reset_buffers();
		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);
		trede.shadeTable = telos_st;
		
		while (timer::getBeat() < end)
		{
			fixed16 beat = timer::getBeat() - start;
			
			trede.setTime(beat*16);
			trede.draw(fb::bb);
			
			fixed16 glowtime = beat*21*2 * (1.0/30);
			if (glowtime>=fixed16(512))glowtime-=fixed16(512);
			
			glow::update(telos_glow, glowtime );
			
			part::swap();
		}

		irqDisable(IRQ_VCOUNT);
		SetMode(MODE_4 | BG2_ON);
		
	}
};
