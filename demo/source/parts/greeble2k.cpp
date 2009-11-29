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

extern const u8  glow_overlay_oam[];
extern const u8  glow_overlay_tileset_raw[];
extern const u16 glow_overlay_tileset_pal[];

extern const u8  clown_oam[];
extern const u8  clown_tileset_raw[];
extern const u16 clown_tileset_pal[];


extern const u8  dot_overlay_oam[];
extern const u8  dot_overlay_tileset_raw[];
extern const u16 dot_overlay_tileset_pal[];

extern const u16 hdr_disco_xpal[];
extern const u8 disco_bg[];
extern const u8 discoenv_tex[];

extern const u8 hdrbump_ebump[];
extern const u8 kulenm_ebump[];
extern const u8 alleyenvsb_tex[];
extern const u8 normalmap_ebump[];

extern const u8 objectcompletemap_tex[];
extern const u8 lopolycompletemap_tex[];
extern const u8 greeble2k_pal[];

namespace scene {
// #include "scene_hdr_fargedisco.h"
#include "scene_greeble2k.h"
};

static int blanks = 0;

static void vblank()
{
	pimp_vblank();
	pimp_frame();

	s32 alpha = (fixed_fsin(blanks << 2) >> (16 - 3)) + 8;
	REG_BLDALPHA = (alpha << 0) | ((16 - alpha) << 8);

	blanks++;
}

static void callback(int type, int data)
{
}

#define OVERLAY(name) fb::overlay(name##_oam, name##_tileset_raw, name##_tileset_pal)

namespace parts
{

	void greeble2k(int len)
	{
		pimp_set_callback(callback);

		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);
//		EnableInterrupt(IE_HBL);


		int swaps = 0;
		blanks = 0;

		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);

		VBlankIntrWait();
		for (int i = 0; i < 128; ++i)
		{
			OAM[i].attr0 = OBJ_DISABLE;
		}

		REG_BLDCNT = 0;
		REG_BLDCNT = BIT(10) | (1 << 6) | BIT(4);
//		REG_BLDALPHA = (8 << 0) | (8 << 8);
//		*(vu16*)(REG_BASE + 0x2) = 1;

//		grid_glow::init(glow_overlay_oam, glow_overlay_tileset_raw, glow_overlay_tileset_pal, hdr_disco_xpal);
//		grid_glow::init(dot_overlay_oam, dot_overlay_tileset_raw, dot_overlay_tileset_pal, pal_xpal);

//		OVERLAY(glow_overlay);

		OVERLAY(clown);
		CpuFastSet(greeble2k_pal, BG_COLORS, 512 / 4);
		SetMode(MODE_4 | BG2_ON | OBJ_ON | OBJ_1D_MAP | (1 << 5));

		const int img_height = 1000;

		int scrolly = 0;
		int flash = 128;
		int start_order = pimp_get_order();
		while (pimp_get_order() - start_order < len)
		{
			scrolly += 4;
			if (scrolly>img_height) scrolly=0;
		
			int h0 = img_height - scrolly;
			if (h0>160) h0=160;
			int h1 = 160-h0;
			if (h1<0) h1=0;
//			CpuFastSet(disco_bg+scrolly*240, fb::bb, COPY32|(h0*240/4));
//			if (h1) CpuFastSet(disco_bg, fb::bb+h0*240, COPY32|(h1*240/4));

			trede.setFrame(swaps+256, 0);
			trede.draw(fb::bb);
 
			int lo = glow::intensity - (glow::range);
			int hi = glow::intensity + (glow::range);
/*  
			if (lo < 0) lo = 0;
			if (hi < lo+0x3000) hi = lo+0x3000;
			int glow_hi = hi;// - (glow::range>>2);// - 0x2000;
			int glow_lo = (lo+glow_hi*3)>>2;//(lo+glow_hi)>>1;
*/
#if 0
			for (int y = 0; y < 10; ++y)
			{
				for (int x = 0; x < 15; ++x)
				{
					u32 pixol = fb::bb[x * 16 + y * 16 * GBA_WIDTH + (8 + 8 * GBA_WIDTH)];
					grid_glow::palette_set[y][x] = 15 - (hdr_disco_xpal[pixol * 4 + 0] >> 12);
				}
			}
#endif

//			exposure::update(lo, hi, glow_lo, glow_hi, hdr_disco_xpal);
//			exposure::update(0, 0x10000, 0, ((hi*3+0x10000)>>2)-0x2000, sxpal);
			OBJ_COLORS[0] = RGB8(255,0,0);
//			glow::update(fb::bb, 0xB000);
			fb::swap();
			swaps++;
		}
		pimp_set_callback(NULL);
	}

#if 0
	void gridglow(int len)
	{
		pimp_set_callback(callback);

		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);
//		EnableInterrupt(IE_HBL);


		int swaps = 0;
		blanks = 0;

		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount, 1);

		VBlankIntrWait();
		for (int i = 0; i < 128; ++i)
		{
			OAM[i].attr0 = OBJ_DISABLE;
		}

		REG_BLDCNT = 0;
		REG_BLDCNT = BIT(10) | (1 << 6) | BIT(4);
//		*(vu16*)(REG_BASE + 0x2) = 1;

		grid_glow::init(glow_overlay_oam, glow_overlay_tileset_raw, glow_overlay_tileset_pal, pal_xpal);
//		grid_glow::init(dot_overlay_oam, dot_overlay_tileset_raw, dot_overlay_tileset_pal, pal_xpal);

//		OVERLAY(glow_overlay);
		SetMode(MODE_4 | BG2_ON | OBJ_ON | OBJ_1D_MAP | (1 << 5));
		
		int flash = 128;
		int start_order = pimp_get_order();
		while (pimp_get_order() - start_order < len)
		{
			static fixed16 time = 0;
			static int last_order = 0;

			int svart=0;
			CpuFastSet(&svart, fb::bb, (240*160/4) | FILL);
			
			trede.setTime(time);
			trede.draw(fb::bb);

			time  += 1.0;
			exposure::update(0x10000, 0xC000, 0x18000, spikebones_xpal);
			BG_COLORS[0] = 0;

			for (int y = 0; y < 10; ++y)
			{
				for (int x = 0; x < 15; ++x)
				{
					u32 pixol = fb::bb[x * 16 + y * 16 * GBA_WIDTH + (8 + 8 * GBA_WIDTH)];
					grid_glow::palette_set[y][x] = 15 - (spikebones_xpal[pixol * 4 + 0] >> 12);
				}
			}

			fb::swap();
			swaps++;
			if (flash > 0) flash -= 8;
		}
		pimp_set_callback(NULL);
	}
#endif
};
