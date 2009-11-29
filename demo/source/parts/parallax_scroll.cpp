#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <gba_affine.h>
#include <stdio.h>
#include "../base.h"
#include "../palette.h"
#include "../part.h"
#include "../tilepool.h"
#include "../fb.h"
#include "../timer.h"
#include "../glow.h"

#define MAP_BASE_BLOCK     28
#define MAP_ADDRESS        MAP_BASE_ADR(MAP_BASE_BLOCK)
#define CHAR_BASE_BLOCK    0

extern const u8  parallax_glow[];
extern const u16 scroll2_pal[];
extern const u8  scroll2_0_tileset_raw[];
extern const u8  scroll2_1_tileset_raw[];
extern const u8  scroll2_2_tileset_raw[];
extern const u8  scroll2_3_tileset_raw[];
extern const u16 scroll2_tile_delta_offsets_bin[];
extern const u16 scroll2_tile_deltas_bin[];

static int offset = 0;
static const u16 *offset_ptr = scroll2_tile_delta_offsets_bin;

#define TILEPOOLS 2
TilePool tilepools[TILEPOOLS] EWRAM_DATA;

#define LAYERS 4
static volatile int layer_scroll[LAYERS] EWRAM_DATA;
static volatile int layer_tempo[LAYERS] EWRAM_DATA;
static const int    layer_pool[LAYERS] = { 0, 0, 0, 0 };
static const u8    *tilesets[LAYERS] = {scroll2_0_tileset_raw, scroll2_1_tileset_raw, scroll2_2_tileset_raw, scroll2_3_tileset_raw};

static void setup_tile(int x, int y, int layer, int tile)
{
	u16 *map_dst = &((u16*)MAP_BASE_ADR(MAP_BASE_BLOCK+layer))[y * 32 + (x & 31)];
	
	int hw_tile = *map_dst;
	hw_tile &= 0xFFF; /* strip away palette number */
	
	/* de-allocate old tile */
	if (hw_tile != 0)
	{
		tilepools[layer_pool[layer]].freeTile(hw_tile);
		hw_tile = 0;
	}

	if (tile != 0)
	{
		hw_tile = tilepools[layer_pool[layer]].getTile();
		u16 *src = (u16*)&tilesets[layer][((8 * 8) / 2) * tile];
		u16 *dst = (u16*)PATRAM4(0, hw_tile);
		CpuFastSet(src, dst, (8 * 8) / 4 / 2);
	}
	
	*map_dst = hw_tile | ((layer & 0xF) << 12);
}

static void reset_scroller()
{
	int svart = 0;
	CpuFastSet(&svart, fb::fb, FILL | (240*160/4));
	CpuFastSet(&svart, fb::bb, FILL | (240*160/4));
	
	offset = 0;
	offset_ptr = scroll2_tile_delta_offsets_bin;
	
	/* prepare tilepools */
	for (int i = 0; i < TILEPOOLS; ++i)
	{
		tilepools[i].reset();
		int clear_tile = tilepools[i].getTile(); // reserve the first tile
		assert(clear_tile == 0);
	
		// make sure clear_tile is really cleared
		{
			u16 *dst = (u16*)PATRAM8(0, clear_tile);
			u32 zero = 0;
			CpuFastSet(&zero, dst, FILL | (8 * 8) / 4);
		}
	}

	/* setup layers */
	for (int i = 0; i < LAYERS; ++i)
	{
		layer_tempo[i]  = (1 << 16) / (1 + i);
		layer_scroll[i] = layer_tempo[i] * 2;
	}
}

static void setup_layer(int layer)
{
	int commands = *offset_ptr++;
	if (commands == 0xFFFF)
	{
		/* loop */
		reset_scroller();
		commands = *offset_ptr++;
	}

	for (int cmd = 0; cmd < commands; ++cmd)
	{
		u32 x    = scroll2_tile_deltas_bin[offset++];
		u32 y    = scroll2_tile_deltas_bin[offset++];
		u32 tile = scroll2_tile_deltas_bin[offset++];
		setup_tile(x, y, layer, tile);
	}
}

#include "../fb.h"

static volatile int hscroll = 0;
static void init()
{
	VBlankIntrWait();
	
	offset = 0;
	hscroll = 2;
	offset_ptr = scroll2_tile_delta_offsets_bin;	
	
	fb::reset();
	
	/* copy palette */
	CpuFastSet(scroll2_pal, BG_COLORS, (2 * 256) / 4);
	BG_COLORS[0] = BG_COLORS[16];
	
	reset_scroller();
	
	/* setup layers */
	for (int i = 0; i < LAYERS; ++i)
	{
		int scroll = layer_scroll[i];
		int inner_scroll = scroll & 7;
		int outer_scroll = scroll / 8;
		
		setup_layer(i);
	}

	// set the screen base to 31 (0x600F800) and char base to 0 (0x6000000)
	BGCTRL[0] = SCREEN_BASE(MAP_BASE_BLOCK + 0) | CHAR_BASE(layer_pool[0]) | BG_SIZE_0 | BG_16_COLOR; // | CHAR_PALETTE(1);
	BGCTRL[1] = SCREEN_BASE(MAP_BASE_BLOCK + 1) | CHAR_BASE(layer_pool[1]) | BG_SIZE_0 | BG_16_COLOR; // | CHAR_PALETTE(0);
	BGCTRL[2] = SCREEN_BASE(MAP_BASE_BLOCK + 2) | CHAR_BASE(layer_pool[2]) | BG_SIZE_0 | BG_16_COLOR; // | CHAR_PALETTE(0);
	BGCTRL[3] = SCREEN_BASE(MAP_BASE_BLOCK + 3) | CHAR_BASE(layer_pool[3]) | BG_SIZE_0 | BG_16_COLOR | CHAR_PALETTE(0);
	SetMode(MODE_0 | BG0_ON | BG1_ON | BG2_ON | BG3_ON | OBJ_ON);

	glow::init();
}

static void vblank()
{
	for (int i = 0; i < LAYERS; ++i)
	{
		layer_scroll[i] = hscroll / (i + 1);
		int scroll = layer_scroll[i];
		
		setup_layer(i);
		
		BG_OFFSET[i].x = scroll;
	}
	hscroll++;
	
	glow::vblank();
}


namespace parts
{
	// NOTE:
	// for at glow skal funke ma farge 0 alltid vaere svart
	void parallax_scroll(fixed16 start, fixed16 end)
	{
		part::set_part(init, vblank);
		
		while (timer::getBeat() < end)
		{
			VBlankIntrWait();
			glow::update(parallax_glow, fixed16(hscroll-4) * (1.0/20) );
			part::swap();
		}
	}
};
