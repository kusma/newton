#include "exstructs.h"

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
#include "fb.h"
#include "config.h"
#include "vcount.h"
#include "palette.h"
#include "3dthen.h"
#include "decomp.h"
#include "flikk.h"
#include "flikkfx.h"
#include "glow.h"
#include "exposure.h"
#include <pimpmobile.h>

extern const u8  skybox_tex[];
extern const u8  skybox2_tex[];
extern const u8  normal_nmap[];
extern const u16 pal_xpal[];

extern const u8  redenv_tex[];
extern const u8  spikelangs_tex[];
extern const u8  spiketvers_tex[];
extern const u16 spikebones_xpal[];

extern const u8  flare_raw[];
extern const u16 flare_pal[];

namespace scene {
//	int pal_subpal = 65 - 16;
	int pal_subpal = 33 + 16;
#include "scene_greeble.h"
//#include "scene_bonespike.h"
};

static int blanks = 0;
static void vblank()
{
	pimp_vblank();
	blanks++;
	pimp_frame();
}

static void callback(int type, int data)
{
}

namespace parts
{
	void greeble(int len)
	{
		pimp_set_callback(callback);
		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);

		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);

		for (int i = 0; i < 127; ++i)
		{
			OAM[i].attr0 = OBJ_DISABLE;
		}

		int sx = 0;
		int sy = 0;

		VBlankIntrWait();
		OAM[0].attr0 = OBJ_Y(sx) | OBJ_256_COLOR | OBJ_MODE(1);
		OAM[0].attr1 = OBJ_X(sy) | OBJ_SIZE(Sprite_64x64);
		OAM[0].attr2 = OBJ_CHAR(512 + 8 * 8 * 2 * 0);

		OAM[1].attr0 = OBJ_Y(sx +  0) | OBJ_256_COLOR | OBJ_MODE(1);
		OAM[1].attr1 = OBJ_X(sy + 64) | OBJ_SIZE(Sprite_64x64);
		OAM[1].attr2 = OBJ_CHAR(512 + 8 * 8 * 2 * 1);

		OAM[2].attr0 = OBJ_Y(sx + 64) | OBJ_256_COLOR | OBJ_MODE(1);
		OAM[2].attr1 = OBJ_X(sy +  0) | OBJ_SIZE(Sprite_64x64);
		OAM[2].attr2 = OBJ_CHAR(512 + 8 * 8 * 2 * 2);

		OAM[3].attr0 = OBJ_Y(sx + 64) | OBJ_256_COLOR | OBJ_MODE(1);
		OAM[3].attr1 = OBJ_X(sy + 64) | OBJ_SIZE(Sprite_64x64);
		OAM[3].attr2 = OBJ_CHAR(512 + 8 * 8 * 2 * 3);

		REG_BLDCNT = BIT(10) | (1 << 6) | BIT(4);
		REG_BLDALPHA = (16) | (16 << 8);

		int swaps = 0;
		blanks = 0;

		fixed16 brightness = 1.f;

		REG_BLDALPHA = (0) | (16 << 8);
		CpuFastSet(flare_pal, OBJ_COLORS, 512 / 4);
		CpuFastSet(flare_raw, BITMAP_OBJ_BASE_ADR, (64 * 64 * 4) / 4);
		
		int flash = 128;
		while (pimp_get_order() < 5)
		{
			static int time = 0;

			if (pimp_get_order()==4 && time<100) time=100; 
/*
			int svart = 0;
			CpuFastSet(&svart, fb::bb, (240 * 160 / 4) | FILL);
*/
			trede.setTime(time);
			trede.draw(fb::bb);
			

			sx = trede.ram_verts[0].x - 64;
			sy = trede.ram_verts[1].y - 64;

			int hehez = trede.ram_lights[2];

//			dprintf("%i\n", hehez);
			if (time >= 100+57 && time < 100+66) brightness = 0;
			else if (time > 100 && time < 100+15) brightness = (brightness * fixed16(0.70)) + (fixed16(0.15));
						//else if (time > 100+97) brightness = (brightness * fixed16(0.5));
			else if (time > 100+50 && time < 100+59) brightness = (brightness * fixed16(0.5));
			else if (time > 100+97) brightness = (brightness * fixed16(0.3));
			else if (time > 100+86) brightness = (brightness * fixed16(0.70)) + (fixed16(0.15));
			//else if (time > 100+50 && time < 100+54) brightness = (brightness * fixed16(0.5));
			else brightness = (brightness * fixed16(0.75)) + fixed16(0.25f);
			
		//	hehez = 0x10000 - (hehez>>8);
		//	if (hehez > 0x10000) hehez = 0x10000;
		//	if (hehez < 0) hehez = 0;
			
			int hehehmm = s64(brightness.get_val()) >> (12);
			if (hehehmm<0) hehehmm = 0;
			if (hehehmm>16) hehehmm = 16;
			REG_BLDALPHA = (hehehmm) | (16 << 8);

			if (sy < 0  ) sy = 0;
			if (sy > 160) sy = 160;

			OAM[0].attr0 = OBJ_Y(sy +  0) | OBJ_256_COLOR | OBJ_MODE(1);
			OAM[0].attr1 = OBJ_X(sx +  0) | OBJ_SIZE(Sprite_64x64);

			OAM[1].attr0 = OBJ_Y(sy +  0) | OBJ_256_COLOR | OBJ_MODE(1);
			OAM[1].attr1 = OBJ_X(sx + 64) | OBJ_SIZE(Sprite_64x64);

			OAM[2].attr0 = OBJ_Y(sy + 64) | OBJ_256_COLOR | OBJ_MODE(1);
			OAM[2].attr1 = OBJ_X(sx +  0) | OBJ_SIZE(Sprite_64x64);

			OAM[3].attr0 = OBJ_Y(sy + 64) | OBJ_256_COLOR | OBJ_MODE(1);
			OAM[3].attr1 = OBJ_X(sx + 64) | OBJ_SIZE(Sprite_64x64);

			if (time != 0)
			{
//				exposure::update(0x10000 - (flash << 9), 0xC000, 0x18000, pal_xpal);
				REG_DISPCNT |= OBJ_ON | OBJ_1D_MAP;
				fb::swap();
			}
/*
			if (time == 0)
			{
				while (1);
			}
*/
			time  += 1;
			swaps++;
			if (flash > 0) flash -= 8;
		}
/*
		consoleInit(0, 4, 0, NULL, 0, 15);
		BG_COLORS[0] = RGB5(0, 0, 0);
		BG_COLORS[241] = RGB5(31, 31, 31);
		REG_DISPCNT = MODE_0 | BG0_ON;

		printf("FPS: %2.3f\n", float(swaps*60) / blanks);
		printf("ms/frame: %2.3f\n", 1000.0 / (float(swaps*60) / blanks));
		while(1);
*/
//		trede.ram_lights[0]
		pimp_set_callback(NULL);
	}

};
