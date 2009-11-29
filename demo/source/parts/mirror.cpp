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

extern const u8 flare_pal[];
extern const u8 flare_raw[];

extern const u8 mirror_st[];
extern const u16 mirror_pal[];

extern const u8 textures1_tex[];
extern const u8 textures2_tex[];
extern const u8 textures3_tex[];
extern const u8 textures4_tex[];
extern const u8 nmcorner_nmap[];
extern const u8 nmmiddle_nmap[];
static const int mirrorpal_subpal = 256-16;

namespace {
namespace scene {
#include "../scenes/mirror.h"
};
};

static int blanks=0;
static void vblank()
{
	blanks++;
}


static int sx = 0;
static int sy = 0;

static void init()
{
	VBlankIntrWait();
	int svart=0;
	CpuFastSet(&svart, BG_COLORS, FILL|(512/4));
	CpuFastSet(&svart, fb::bb, FILL|(240*160/4));
	CpuFastSet(&svart, fb::fb, FILL|(240*160/4));		
	CpuFastSet(mirror_pal, BG_COLORS, (512 / 4));
	fb::setMode(MODE_4 | BG2_ON );
	for (unsigned i = 0; i < 128; i++) OAM[i].attr0 = OBJ_DISABLE;
}


namespace parts
{

	void mirror(fixed16 start, fixed16 end)
	{
		part::set_part(init, vblank);
		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount, 2);
		trede.shadeTable = mirror_st;
		

		int swaps = 0;
		blanks = 0;
		
		while (timer::getBeat() < end)
		{
			fixed16 beat = timer::getBeat() - start;
			
			// round the time to avoid interpolation
			int frame = (beat*8).get_val()>>16;
			trede.setFrame(frame,0);

			trede.draw(fb::bb);
			sx = trede.ram_lights[0] - 64;
			sy = trede.ram_lights[1] - 64;
			OAM[0].attr0 = OBJ_Y(sy +  0) | OBJ_256_COLOR | OBJ_MODE(1);
			OAM[0].attr1 = OBJ_X(sx +  0) | OBJ_SIZE(Sprite_64x64);

			OAM[1].attr0 = OBJ_Y(sy +  0) | OBJ_256_COLOR | OBJ_MODE(1);
			OAM[1].attr1 = OBJ_X(sx + 64) | OBJ_SIZE(Sprite_64x64);

			OAM[2].attr0 = OBJ_Y(sy + 64) | OBJ_256_COLOR | OBJ_MODE(1);
			OAM[2].attr1 = OBJ_X(sx +  0) | OBJ_SIZE(Sprite_64x64);

			OAM[3].attr0 = OBJ_Y(sy + 64) | OBJ_256_COLOR | OBJ_MODE(1);
			OAM[3].attr1 = OBJ_X(sx + 64) | OBJ_SIZE(Sprite_64x64);

			part::swap();
			swaps++;
		}
	}
};
