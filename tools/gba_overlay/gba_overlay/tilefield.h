#pragma once

#include "tile.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160
#define TILEFIELD_WIDTH (SCREEN_WIDTH / TILE_WIDTH)
#define TILEFIELD_HEIGHT (SCREEN_HEIGHT / TILE_HEIGHT)

#include <assert.h>
#define ASSERT assert

class TileField
{
public:
	Tile &operator()(int x, int y)
	{
		ASSERT(x >= 0);
		ASSERT(x <  TILEFIELD_WIDTH);
		ASSERT(y >= 0);
		ASSERT(y <  TILEFIELD_HEIGHT);
		return data[y][x];
	}
	
	const Tile &operator()(int x, int y) const
	{
		ASSERT(x >= 0);
		ASSERT(x <  TILEFIELD_WIDTH);
		ASSERT(y >= 0);
		ASSERT(y <  TILEFIELD_HEIGHT);
		return data[y][x];
	}
	
private:
	Tile data[TILEFIELD_HEIGHT][TILEFIELD_WIDTH];
};

