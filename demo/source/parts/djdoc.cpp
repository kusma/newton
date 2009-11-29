#include "exstructs.h"

#include <pimpmobile.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include "fb.h"
#include "config.h"
#include "vcount.h"
#include "palette.h"
#include "3dthen.h"
#include "decomp.h"
#include "flikk.h"
#include "flikkfx.h"
#include "noise.h"

// annet stuff
extern const u16 brushlb_mask[];
extern const u16 shitface_pal[];
extern const u8 shitface_spr[];
extern const u8 shitface_oam[];
 
extern u16 parallax_pal[];
extern u8  parallax_height_raw[];
extern u8  parallax_raw[];

extern u16 test_mask[];
extern u16 kvakk_flikk[];
extern u16 sfc_flikk[];
extern u16 djdoc_flikk[];
extern u16 street_flikk[];

extern u8  flikkmask_raw[];
extern u16 flikkmask_pal[];




extern const u16 shit_mask[];

extern const u8  street1_oam[];
extern const u8  street1_tileset_raw[];
extern const u16 street1_tileset_pal[];

extern const u8  street2_oam[];
extern const u8  street2_tileset_raw[];
extern const u16 street2_tileset_pal[];

extern const u8  street3_oam[];
extern const u8  street3_tileset_raw[];
extern const u16 street3_tileset_pal[];

static int blanks=0;
static volatile int fade = 256;
static const u16 *pal = street1_tileset_pal;
static volatile bool dick = false;

static void vblank()
{
	pimp_vblank();
	blanks++;
	//noise::update();
	palette::boost(OBJ_COLORS, pal, 256, -256+fade);
	
	if (!dick) {
		FORI(16) {
			int v = (cheap_rand() & (0x1F>>3)) + 5;
			BG_COLORS[i + 16] = RGB5(v,v,v);
		}
	}
	if (fade > 0) fade -= 8;
	pimp_frame();
}

static void callback(int type, int data)
{
	fade = 256;
	REG_DISPCNT |= OBJ_ON | OBJ_1D_MAP;
	switch (data)
	{
		case 0: fb::overlay(street1_oam, street1_tileset_raw, street1_tileset_pal); pal = street1_tileset_pal; break;
		case 1: fb::overlay(street2_oam, street2_tileset_raw, street2_tileset_pal); pal = street2_tileset_pal; break;
		case 2: fb::overlay(street3_oam, street3_tileset_raw, street3_tileset_pal); pal = street3_tileset_pal; break;
		case 3: dick =true; break;
	}
}

extern const char penis_raw[];
extern const u16  penis_pal[];

namespace parts
{
	void djdoc(int flikkvalg)
	{
		int startorder = pimp_get_order();
		pimp_set_callback(callback);


		
		Flikk flikk(djdoc_flikk);
		FORI(16) {
			int v = (i<<4) | i;
			BG_COLORS[i] = RGB8(v,v,v);
		}

	//	noise::setup();
		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);

		REG_BLDCNT = BIT(10) | (1 << 6) | BIT(4);
		REG_BLDALPHA = (16) | (16 << 8);

		int swaps=0;
		int frame=0;
		while (pimp_get_order() < startorder + 2)
		{
			if (!dick)
			{
				/* TODO: ping pong */
				flikk.nextFrame();
				if (flikk.getFrame()==0) flikk.nextFrame(); // deterministisk bug i frame 0 i stuff lagra i AA. pussig.
				FORI(240*160/4) {
					((u32*)fb::bb)[i] = (cheap_rand() & 0x0F0F0F0F) + 0x10101010;
				}
				flikk.drawMov(fb::bb, 5);

				fb::waitline(159);
				fb::waitline(160);
				fb::waitline(159);
				fb::waitline(160);
				fb::waitline(159);
				fb::waitline(160);

				fb::swap();

				swaps++;
			}
			else
			{
//				REG_BLDCNT = 0;
				CpuFastSet(penis_pal, BG_COLORS, 512 / 4);
				CpuFastSet(penis_raw, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
				fb::swap();
				VBlankIntrWait();
			}
		}
	}
};
