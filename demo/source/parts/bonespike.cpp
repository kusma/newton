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

// annet stuff
extern const u16 brushlb_mask[];
extern const u16 shitface_pal[];
extern const u8 shitface_spr[];
extern const u8 shitface_oam[];

// kuber stuff
extern const u16 kuber_pal[];
extern const u8  kubercomplete_tex[];
extern const u8  kubersb_tex[];
extern const u8  kubersbdn_tex[];


extern u16 parallax_pal[];
extern u8  parallax_height_raw[];
extern u8  parallax_raw[];

extern u16 test_mask[];
extern u16 yeah_flikk[];


extern u8  flikkmask_raw[];
extern u16 flikkmask_pal[];

extern const u16 spikebones_shadow_flikk[];
extern const u16 spikebones2_shadow_flikk[];
extern const u8  hull06s_tex[];
extern const u8  redenv_tex[];
extern const u8  spikelangs_tex[];
extern const u8  spiketvers_tex[];
extern const u16 spikebones_xpal[];
extern const u8  bgcompletemap_tex[];
extern const u8 bjelkercompletemap_tex[];

extern const u8  headbang1_oam[];
extern const u8  headbang1_tileset_raw[];
extern const u16 headbang1_tileset_pal[];

extern const u8  headbang2_oam[];
extern const u8  headbang2_tileset_raw[];
extern const u16 headbang2_tileset_pal[];


namespace scene {
#include "scene_bonespike.h"
};

namespace scene2 {
#include "scene_bonespike2.h"
};

extern const u16 shit_mask[];

static volatile int blanks = 0;

static volatile int flash = 256;
static volatile int time_adder = 0;
static volatile int fade = 0;
static volatile int fade_speed = 0;

static void vblank() {
	pimp_vblank();
	blanks++;
	flash -= 8;
	fade += fade_speed;
	if (flash < 0) flash = 0;
	pimp_frame();
}

static void callback(int type, int data)
{
	switch (data)
	{
		case 0: fb::overlay(headbang1_oam, headbang1_tileset_raw, headbang1_tileset_pal); break;
		case 1: fb::overlay(headbang2_oam, headbang2_tileset_raw, headbang2_tileset_pal); break;
		case 2: {
			flash = 512; 
			time_adder += 8;
			break;
		}
		case 3: {
			flash = 256; 
			time_adder += 5;
			break;
		}
	}
}

namespace parts {

	void bonespike(int len)
	{
		pimp_set_callback(callback);
		int startorder = pimp_get_order();

		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);

		if (len == 0)
		{
			static u16 temp[256] EWRAM_DATA;
			Flikk flikk(spikebones_shadow_flikk);
			Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);
			exposure::update(0x10000, 0xC000, 0x18000, spikebones_xpal, temp);
//			exposure::update(0x10000, 0xC000, 0x18000, spikebones_xpal);		
			int swaps=0;
			blanks=0;
			s32 thresh = 0x10000;
			while (pimp_get_order() < startorder + 2)
			{
				static fixed16 time = 0;
				time = fixed16(1.0/6) * blanks;
				int svart=0;
				CpuFastSet(&svart, fb::bb, (240*160/4) | FILL);
				trede.setTime(time);
				flikk.setFrame(time);
				trede.draw(fb::bb);
				flikk.drawShift(fb::bb);

				int f = flash >> 4;
				palette::add(BG_COLORS, temp, 256, RGB5(f,f,f));

				fb::swap();
				swaps++;
			}
		}

		if (len == 1)
		{
			static u16 temp[256] EWRAM_DATA;
			static u16 temp2[256] EWRAM_DATA;

			Flikk flikk(spikebones2_shadow_flikk);
			Trede trede(scene2::verts, scene2::vertcount, scene2::faces, scene2::facecount, scene2::uvs, scene2::materials, scene2::framestates, scene2::framecount);
//			exposure::update(0x10000, 0xC000, 0x18000, spikebones_xpal);
			exposure::update(0x10000, 0xC000, 0x18000, spikebones_xpal, temp);

//			fb::overlay(headbang1_oam, headbang1_tileset_raw, headbang1_tileset_pal);
//			fb::overlay(headbang2_oam, headbang2_tileset_raw, headbang2_tileset_pal);
//			REG_BLDCNT = 0;
			REG_BLDCNT = 0;
			REG_BLDALPHA = 0;


			CpuFastSet(BG_COLORS, temp, 256 / 4);
			int swaps=0;
			blanks=0;
			s32 thresh = 0x10000;
			time_adder = 0;
			while (pimp_get_order() < startorder + 3)
			{
				fixed16 time = swaps + time_adder;
				trede.setTime(time);
				flikk.setFrame(time);
				trede.draw(fb::bb);
				flikk.drawShift(fb::bb);
				if (pimp_get_order() == startorder + 2)
				{
					fade_speed = 1;
					REG_DISPCNT &= ~OBJ_ENABLE;
					if ((fade >> 1) < 32)
					{
						palette::colorize(temp2, temp, 256, RGB5(31, 31, 31), (fade >> 1));
						palette::add(BG_COLORS, temp2, 256, RGB5(31 - (fade >> 1), 31 - (fade >> 1), 31 - (fade >> 1)));
					}
					else
					{
						palette::colorize(temp2, temp, 256, RGB5(31, 31, 31), 31);
						int fjall = (32 - (fade >> 1)) << 3;
						if (fjall < -255) fjall = -255;
						palette::boost(BG_COLORS, temp2, 256, fjall);
					}
				}
				else
				{
					int f = flash >> 4;
					palette::add(BG_COLORS, temp, 256, RGB5(f,f,f));
				}

				fb::swap();
				swaps++;
				if ((pimp_get_order() == startorder + 2) && (pimp_get_row() >= 24)) return;
			}
		}

		/*
		consoleInit(0, 4, 0, NULL, 0, 15);
		BG_COLORS[0] = RGB5(0, 0, 0);
		BG_COLORS[241] = RGB5(31, 31, 31);
		REG_DISPCNT = MODE_0 | BG0_ON;
		printf("FPS: %2.3f\n", float(swaps*60) / blanks);
		while(1);
		*/
		DisableInterrupt(IE_VCNT);
	}

};
