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
#include "../vcount.h"
#include "../palette.h"
#include "../3dthen.h"
#include "../decomp.h"
#include "../flikk.h"
#include "../flikkfx.h"
#include "../glow.h"
#include "../exposure.h"
#include "../part.h"

extern const u8 growy_tex[];
extern const u8 flextextiles_tex[];
extern const u16 part_growy_pal[];

namespace scene {
#include "../scenes/scene_growy.h"
};


static int blanks = 0;
static void vblank()
{
	blanks++;
}

static void init()
{
	VBlankIntrWait();
	
	for (int i = 0; i < 128; ++i)
	{
		OAM[i].attr0 = OBJ_DISABLE;
	}

	REG_BLDCNT = 0;

	SetMode(MODE_4 | BG2_ON | OBJ_ON | OBJ_1D_MAP | (1 << 5));

	palette::make_fadepal(BG_COLORS, part_growy_pal, RGB5(31, 31, 31));
	BG_COLORS[0] = RGB5(31, 31, 31);
	
}

#define OVERLAY(name) fb::overlay(name##_oam, name##_tileset_raw, name##_tileset_pal)

namespace parts
{

	void bboy(int len)
	{
		part::set_part(init, vblank);

		int swaps = 0;
		blanks = 0;

		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);

		while (blanks < 60 * 4 * 4)
		{
			u32 black = 0;
//			CpuFastSet(&black, fb::bb, FILL|COPY32|((120 * 240)/4));

			for (int y = 0; y < GBA_HEIGHT; ++y)
			{
				u8 *dst = &fb::bb[y * GBA_WIDTH];
				u32 col = y + blanks;
				u8 *src = (u8*)&col;
				CpuFastSet(src, dst, FILL | COPY32 | (GBA_WIDTH / 4));
			}
			
			part::swap();
			swaps++;
		}
	}
};
