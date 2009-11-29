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
#include "math.h"

// data
extern const u16 rot_pal[];

extern const u8  rot1_raw[];
extern const u16 rot1_map[];
extern const u8  rot2_raw[];
extern const u16 rot2_map[];
extern const u8  rot3_raw[];
extern const u16 rot3_map[];

extern const u8  dialog1_oam[];
extern const u8  dialog1_tileset_raw[];
extern const u16 dialog1_tileset_pal[];

extern const u8  dialog2_oam[];
extern const u8  dialog2_tileset_raw[];
extern const u16 dialog2_tileset_pal[];

extern const u8  dialog3_oam[];
extern const u8  dialog3_tileset_raw[];
extern const u16 dialog3_tileset_pal[];

extern const u8  dialog4_oam[];
extern const u8  dialog4_tileset_raw[];
extern const u16 dialog4_tileset_pal[];

#define FRAME_TILESET_SIZE 1184
#define FRAME_MAP_SIZE     (32 * 2 * 4)
#define FRAME_FLIPS        4
#define BASE               31
#define MAP_ADDRESS        MAP_BASE_ADR(BASE)

void set_frame(u32 frame, u32 segment, u32 type, bool hflip, bool vflip)
{
	frame &= 31;

	const u16 *map;
	const u8  *tileset;
	switch (type)
	{
		case 0:
			map = rot1_map;
			tileset = rot1_raw;
		break;
		case 1:
			map = rot2_map;
			tileset = rot2_raw;
		break;
		case 2:
			map = rot3_map;
			tileset = rot3_raw;
		break;
		case 3:
			map = rot3_map;
			tileset = rot3_raw;
		break;
	}

	// tileset
	CpuFastSet(
		tileset + frame * FRAME_TILESET_SIZE,
		(u8*)VRAM + segment * FRAME_TILESET_SIZE,
		(FRAME_TILESET_SIZE / 4) | COPY32
	);

	int flips = (hflip ? 1 : 0) | (vflip ? 2 : 0);

	const u32 offs = segment * (FRAME_TILESET_SIZE / 32);
	// map
	const u16 *src = ((const u16*)map) + (frame * FRAME_MAP_SIZE + flips* (FRAME_MAP_SIZE / FRAME_FLIPS));
	u16 *dst = ((u16*)MAP_ADDRESS) + segment * (FRAME_MAP_SIZE / FRAME_FLIPS);
	for (int i = (FRAME_MAP_SIZE / FRAME_FLIPS) / 4; i; --i)
	{
		u32 temp;
		temp = *src++;
		*dst++ = (temp & ~1023) | ((temp & 1023) + offs);
		temp = *src++;
		*dst++ = (temp & ~1023) | ((temp & 1023) + offs);
		temp = *src++;
		*dst++ = (temp & ~1023) | ((temp & 1023) + offs);
		temp = *src++;
		*dst++ = (temp & ~1023) | ((temp & 1023) + offs);
	}
}

static int blanks = 0;

volatile u32 band = 0;
#if 0
static void int_vcount()
{
	while ((REG_DISPSTAT & LCDC_HBL_FLAG) == 0);

	int y = REG_VCOUNT;
	if ((y + 16) < 160) REG_DISPSTAT = (REG_DISPSTAT & 0xFF) | VCOUNT(y + 16);
//	BG2HOFS = fixed_fsin((band << 6) + (blanks << 3)) >> 11;
	band++;
}
#endif

typedef struct
{
	int rot;
	int rot_speed;
} segment_state;
segment_state segs[16];

static u32 pos;
static s32 frame;
static int boost = 0;
static int startorder;
static int vpos = 0;
static int vpos_time = 0;

static void setup_twister()
{
	frame = blanks;

	for (int i = 0; i < 16; ++i)
	{
		segs[i].rot += segs[i].rot_speed;
	}

	for (int i = 0; i < 5; ++i)
	{
		int rot = segs[pos & 15].rot >> 2;
		set_frame(rot, i, (pos & 3), ((pos & 4) == 0), ((pos & 2) == 0));
		pos++;
	}
/*	int_vcount();
	REG_DISPSTAT = (REG_DISPSTAT & 0xFF) | VCOUNT(15 - (vpos % 16)); */
}

static void setup_twister2()
{
	for (int i = 5; i < 11; ++i)
	{
		int rot = segs[pos & 15].rot >> 2;
		set_frame(rot, i, (pos & 3), ((pos & 4) == 0), ((pos & 2) == 0));
		pos++;
	}
}

volatile int last_row = 0;
volatile static int overlay = 0;
static int flash = 256;
static void vblank()
{
	pimp_vblank();
	blanks++;
//	blanks += -boost >> 2;
	if (boost > 0) boost--;

/*
	int b = (boost * boost) >> 5;
	palette::add(BG_COLORS, rot_pal, 16, RGB5(b, b, b));
*/
//	BG_COLORS[15] = RGB5(31, 0, 0);
//	BG_COLORS[15] = RGB5(0, 31, 0);
//	BG_COLORS[15] = RGB5(0, 0, 0);

	vpos = (vpos_time >> 1) + (fixed_fsin(vpos_time) >> 9);
	BG2VOFS = unsigned(vpos) % 16;
	pos = unsigned(vpos) / 16;
	band = pos;

	setup_twister();

	if (pimp_get_order() == startorder)
	{
//		palette::boost(BG_COLORS, rot_pal, 16, 128);
		int amt = 31 - pimp_get_row();
		if (amt > 0) palette::add(BG_COLORS, rot_pal, 16, RGB5(amt, amt, amt));
	}
	else
	{
		static bool first = true;
		if (first)
		{
			first = false;
			for (int i = 0; i < 16; ++i)
			{
				segs[i].rot_speed = (((1 + i * 3) * ((((i >> 1) & 1) == 1) ? -2 : 1) * (((i & 5) != 0) ? 2 : 1)) & 7) + 1;
			}
		}

		int amt = flash >> 4;
		if (amt > 0) palette::add(BG_COLORS, rot_pal, 16, RGB5(amt, amt, amt));
		if (flash > 0) flash -= 16;


		int row = pimp_get_row() - last_row;
		if (row >= 32 - 2)
		{
			REG_DISPCNT &= ~OBJ_ENABLE;
		}
		else if ((row < 4 + 2) || (row >= 32 - 4))
		{
			if (((blanks >> 1) & 1) != 0) REG_DISPCNT &= ~OBJ_ENABLE;
			else REG_DISPCNT |= OBJ_ENABLE;
		}
		else
		{
			REG_DISPCNT |= OBJ_ENABLE;
		}
		vpos_time++;
	}

	pimp_frame();
	setup_twister2();
}

extern const char sfc1_raw[];
extern const u16  sfc1_pal[];
extern const char sfc2_raw[];
extern const u16  sfc2_pal[];
extern const char sfc3_raw[];
extern const u16  sfc3_pal[];

static volatile int sfc_pic = 0;
static const u16 *pal = sfc1_pal;

static void callback(int type, int data)
{
	boost = 256;

	if (data < 4)
	{
		switch (data)
		{
			case 0: fb::overlay(dialog1_oam, dialog1_tileset_raw, dialog1_tileset_pal); break;
			case 1: fb::overlay(dialog2_oam, dialog2_tileset_raw, dialog2_tileset_pal); break;
			case 2: fb::overlay(dialog3_oam, dialog3_tileset_raw, dialog3_tileset_pal); break;
			case 3: fb::overlay(dialog4_oam, dialog4_tileset_raw, dialog4_tileset_pal); break;
		}

		overlay = data;
		REG_DISPCNT |= OBJ_ON;
		last_row = pimp_get_row();
	}
	else
	{
		for (int i = 0; i < 16; ++i)
		{
			if (cheap_rand() & 1) segs[i].rot_speed = - segs[i].rot_speed;
		}
	}
}

void do_pal()
{
	palette::boost(BG_COLORS, pal, 256, boost - 256);
}

volatile static bool need_swap = true;
static void vblank2()
{
	pimp_vblank();

	switch (sfc_pic)
	{
	case 0:
		pal = sfc1_pal;
		CpuFastSet(sfc1_raw, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
	break;
	case 1:
		pal = sfc2_pal;
		CpuFastSet(sfc2_raw, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
	break;
	case 2:
		pal = sfc3_pal;
		CpuFastSet(sfc3_raw, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
	break;
	}
	do_pal();

	blanks++;
	if (boost > 0) boost -= 8;
	if (need_swap) fb::swap();
	need_swap = false;
	pimp_frame();
}

static void callback2(int type, int data)
{
	boost = 256;
	sfc_pic = data;
	need_swap = true;
}

namespace parts {

	void twister(int len)
	{
		startorder = pimp_get_order();
		pimp_set_callback(callback);

		// set the screen base to 31 (0x600F800) and char base to 0 (0x6000000)
		BGCTRL[2] = SCREEN_BASE(BASE);

		vpos = (vpos_time >> 1) + (fixed_fsin(vpos_time) >> 9);
		BG2VOFS = unsigned(vpos) % 16;
		pos = unsigned(vpos) / 16;
		band = pos;
		for (int i = 0; i < 16; ++i)
		{
			segs[i].rot = 0; // pos << 4;
			segs[i].rot_speed = 1 * ((((pos) & 4) == 0) ? 1 : -1);

			pos++;
#if 0		
			// (i - fixed_fsin(i) >> 12) & 3;//i;
			(((1 + i * 3) * ((((i >> 1) & 1) == 1) ? -2 : 1) * (((i & 5) != 0) ? 2 : 1)) & 7) + 1;
//			>> 2;
#endif
		}

		/*
			1024 tiles i 16c modus
			256 / 8 = 32 tiles i bredden
			plass til (1024 / 32) / 2 = 16 unike frames uten optimaliseringer.
			10 synlige frames i høyden. jøss, alt går glætt uten optimaliseringer, gitt...
			men mirroring kan likevel hjelpe på at vi bare har fire ulike segment-typer. vi kan mirrore to veier.
		*/

		/* mirrora h / v = 2 x 2 = 4 forskjellige "looks" pr segment */
		/* 4 (2 x 2 mirror) x 4 */


		VBlankIntrWait();
		// screen mode & background to display
		SetMode( MODE_0 | BG2_ON | OBJ_1D_MAP);
		CpuFastSet(rot_pal, BG_COLORS, 512 / 4);

/*		REG_DISPSTAT = (REG_DISPSTAT & 0xFF) | VCOUNT(0);
		SetInterrupt(IE_VCNT, int_vcount);
		EnableInterrupt(IE_VCNT); */
		REG_BLDCNT = 0;

		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);

		unsigned swaps = 0;
		while (pimp_get_order() < startorder + 3)
		{
			VBlankIntrWait();
			swaps++;
		}

		SetInterrupt(IE_VBL, vblank2);
		EnableInterrupt(IE_VBL);
		pimp_set_callback(callback2);

		fb::reset();
		SetMode(MODE_4 | BG2_ON);
	
		boost = 0;
		sfc_pic = 0;

		while (pimp_get_order() < startorder + 4)
		{
			VBlankIntrWait();
/*			CpuFastSet(sfc1_pal, BG_COLORS, 512 / 4); */
/*			CpuFastSet(sfc1_raw, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4); */
			swaps++;
		}
	}

};
