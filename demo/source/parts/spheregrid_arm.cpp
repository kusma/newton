#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <gba_affine.h>
#include <stdio.h>
#include "../palette.h"
#include "../part.h"
#include "../fb.h"
#include "../math.h"
#include "../decomp.h"
#include "../timer.h"

extern u16 sprite_pal[256];
extern u8  sprite_raw[];

extern u8 spheregrid_overlay1_raw[];
extern u8 spheregrid_overlay2_raw[];
extern u8 spheregrid_overlay3_raw[];

extern u32 offsets[64];
extern u8  sphere00[64];
extern u8 *spheremaps[];

extern u8  sphere_tex[];
extern u16 sphere_pal[];

#define ANGLE_COUNT (128 * 2)

int yflip = false;
/*
void update_sphere(u8 *dst_, int slong_offs, int xrot) __attribute__((section(".iwram1"), long_call));
void downsample_tiles(u8 *src_, u8 *dst_, u32 tile_pitch) __attribute__((section(".iwram1"), long_call));
*/
void update_sphere(u8 *dst_, int slong_offs, int xrot)
{
	/* address:
	YYYXXXyyyxxx
	*/
	u32 *offset = offsets;

#if 0 
	/* debug */
	slong_offs  = 0;
	xrot >>= 2;
	xrot &= 2;
	xrot = 255 + xrot;
#endif
	
	slong_offs &= 255;
	xrot &= 255;


	/* detect first flip */
	int flip1 = (xrot & (ANGLE_COUNT / 2)) != 0;
	xrot &= (ANGLE_COUNT / 2) - 1;

	/* detect second flip */
	int flip2 = (xrot & (ANGLE_COUNT / 4)) != 0;
	xrot &= (ANGLE_COUNT / 4) - 1;

	/* calculate xor masks */
	int slong_mask = 0;
	if (0 != (flip1 ^ flip2)) slong_mask = 255;

	int slatt_mask = 0;
	if (0 != flip1)
	{
		slatt_mask = 255;
		slong_offs += 128;
	}

	if (0 != flip2)
	{
		/* TODO: avoid a gap when flip2 goes high by making the LUT one element bigger, and changing the subtraction below to ANGLE_COUNT / 4 - xrot */
		xrot = ANGLE_COUNT / 4 - xrot;
		slong_offs += 128; 
	}
	
	yflip = false;
	int ymask = 0;
	if (0 != flip2)
	{
		yflip = true;
		ymask = 63;
	}
	
	int xor_mask = slong_mask | (slatt_mask << 8);
	
	u8 *tex = sphere_tex;
	tex += slong_offs;

//	u8 *frame = zoom_tables[xrot];
	u16  *spheremap = (u16*)spheremaps[xrot];

	for (int y_tile = 0; y_tile < 8; ++y_tile)
	{
		for (int y = 0; y < 8; ++y)
		{
			u32 offs = *offset++;
			
			u32 *ldst = (u32*)dst_ + y;
			u32 *rdst = (u32*)dst_ + y + (((8 * 8) / 2) * 7) / 4;
			
			u32 xoffs = (offs >> 3) << 3;
			ldst += xoffs;
			rdst -= xoffs;
			
			#define INNER                        \
			{                                    \
				int uv = *spheremap++;           \
				uv ^= xor_mask;                  \
				                                 \
				u32 lcol = tex[uv];              \
				lline >>=  4;                    \
				lline |= lcol << (32 - 4);       \
				                                 \
				u32 rcol = tex[uv ^ 0xFF];       \
				rline <<=  4;                    \
				rline |= rcol;                   \
			}
			u32 lline = 0;
			u32 rline = 0;
			int x = 0;
			int x_tile = offs >> 3;
			
			switch (offs & 7)
			{
				for (x_tile = 0; x_tile < 4; ++x_tile)
				{
					case 0:
						INNER
					case 1:
						INNER
					case 2:
						INNER
					case 3:
						INNER
					case 4:
						INNER
					case 5:
						INNER
					case 6:
						INNER
					case 7:
						INNER
					
					/* plot line */
					*ldst = lline;
					*rdst = rline;
					
					/* move to next tile in the x direction */
					ldst += ((8 * 8) / 2) / 4;
					rdst -= ((8 * 8) / 2) / 4;
				}
			}
		}
		dst_ += ((8 * 8) / 2) * 8;
	}
}

void downsample_tiles(u8 *src_, u8 *dst_, u32 tile_pitch)
{
	u32 *src = (u32*)src_;
	u32 *dst = (u32*)dst_;
	
	tile_pitch -= (8 * 8) / 2;
	tile_pitch /= 4;
	
	/* for each tile to scale down in the y-direction */
	for (int i = 0; i < 2; ++i)
	{
		/* for each second scanline */
		for (int y = 0; y < 4; ++y)
		{
			/* fetch scanline from first tile. this will be the 4 leftmost pixels in the result */
			u32 line = *src;

			/* shuffle AxBxCxDx -> 0000ABCD (ie, remove every second pixel) */
			line &= 0x0f0f0f0f;
			line += line << 4;
			line = 0x00FF00FF & (line >> 4);
			line += line << 8;
			line = 0xFFFF & (line >> 8);
			
			/* store result for later */
			u32 pix = line;

			/* fetch scanline from second tile. this will be the 4 rightmost pixels in the result */
			line = *(src + (((8 * 8) / 2) / 4));

			/* shuffle AxBxCxDx -> 0000ABCD (ie, remove every second pixel) */
			line &= 0x0f0f0f0f;
			line += line << 4;
			line = 0x00FF00FF & (line >> 4);
			line += line << 8;
			line = 0xFFFF & (line >> 8);
			
			/* compose result */
			pix |= line << 16;
			
			*dst++ = pix;
			src += 2; /* advance two lines (skip every second) */
		}
		/* advance to next tile downwards */
		src += tile_pitch;
	}
}
