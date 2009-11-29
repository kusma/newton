#include "tile.h"
#include "tilefield.h"

#include <stdio.h>
#include <string.h>

Tile::Tile(unsigned topleft_x, unsigned topleft_y)
	:
	topleft_x(topleft_x), topleft_y(topleft_y),
	scale_x(0), scale_y(0),
	enabled(false)
{
	/* nuthin' */
}

Tile::Tile(
	const void *src, int src_pitch,
	int topleft_x, int topleft_y,
	int scale_x, int scale_y
) :
	topleft_x(topleft_x), topleft_y(topleft_y),
	scale_x(scale_x), scale_y(scale_y),
	enabled(true)
{
	int sx = 1 << scale_x;
	int sy = 1 << scale_y;
	
	unsigned char *dst2 = (unsigned char *)data;
	unsigned char *src2 = (unsigned char *)src;
	
	for (int y = 0; y < (TILE_HEIGHT >> scale_y); ++y)
	{
		for (int x = 0; x < (TILE_WIDTH >> scale_x); ++x)
		{
			int accum[4];
			accum[0] = accum[1] = accum[2] = accum[3] = 0;

			int sx2 = x << scale_x;
			int sy2 = y << scale_y;
		
			int count = 0;
			for (int ty = 0; ty < sy; ++ty)
			{
				for (int tx = 0; tx < sx; ++tx)
				{
					int offs = (sx2 + topleft_x + tx) * 4 + (sy2 + topleft_y + ty) * src_pitch;

					int r = src2[offs + 0];
					int g = src2[offs + 1];
					int b = src2[offs + 2];
					int a = src2[offs + 3];

					if ((r != 0) || (g != 255) || (b != 0))
					{
						accum[0] += r;
						accum[1] += g;
						accum[2] += b;
						accum[3] += a;
						count++;
					}
				}
			}
			
			if (count != 0)
			{
				*dst2++ = (unsigned char)(accum[0] / count);
				*dst2++ = (unsigned char)(accum[1] / count);
				*dst2++ = (unsigned char)(accum[2] / count);
				*dst2++ = (unsigned char)(accum[3] / count);
			}
			else
			{
				*dst2++ = 0;
				*dst2++ = 255;
				*dst2++ = 0;
				*dst2++ = 0;
			}
		}
	}
}

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

void Tile::draw(unsigned *dst, int pitch) const
{
	int min_x = MAX(0, topleft_x);
	int min_y = MAX(0, topleft_y);
	
	int max_x = MIN(SCREEN_WIDTH,  topleft_x + TILE_WIDTH);
	int max_y = MIN(SCREEN_HEIGHT, topleft_y + TILE_HEIGHT);

	if (!enabled) return;
	
	unsigned iy = 0;
	for (int y = min_y; y < max_y; ++y, ++iy)
	{
		unsigned *line = (unsigned*)(((char*)dst) + y * pitch);
		
		unsigned ix = 0;
		for (int x = min_x; x < max_x; ++x, ++ix)
		{
			line[x] = data[(ix >> scale_x) + (iy >> scale_y) * (TILE_WIDTH >> scale_x)];
		}
	}
}

/* plot all 8x8 tiles in sprite to dst */
void Tile::plotTiles(unsigned *dst, int tile_offset, int pitch) const
{
	dst = (unsigned*)((char*)dst + tile_offset * 8 * pitch);
	
	int sprite_w = TILE_WIDTH >> scale_x;
	int sprite_h = TILE_HEIGHT >> scale_y;
	
	int x_tiles = sprite_w / 8;
	int y_tiles = sprite_h / 8;
	
	printf("store tiles: %i %i\n", tile_offset, tile_offset + x_tiles * y_tiles);
	
	for (int t = 0; t < x_tiles * y_tiles; ++t)
	{
		int tile = t;
		
		int tile_x = t % x_tiles;
		int tile_y = t / x_tiles;
		
		const unsigned *src = data + tile_x * 8 + tile_y * sprite_w * 8;
		
		unsigned *dst2 = (unsigned*)((char*)dst + 8 * pitch * tile);
		
		for (int y = 0; y < 8; ++y)
		{
			memcpy(dst2, src, 8 * 4);
			src  += sprite_w;
			dst2 = (unsigned*)(((char*)dst2) + pitch);
		}
	}
}
