#pragma once

#include "tile.h"
#include <assert.h>

class TileSet
{
public:
	TileSet() {}

	void setTile(int pos, const Tile &td)
	{
		assert(pos >= 0);
		if (pos >= int(tiles.size())) tiles.resize(pos + 1);
		tiles[pos] = td;
	}

	int findTile(const Tile &tile)
	{
		for (int i = 0; i < int(getTileCount()); ++i)
		{
			if (getTile(i) == tile) return i;
		}
		return -1;
	}


	int addTile(const Tile &td)
	{
		int pos = int(getTileCount());
		setTile(pos, td);
		return pos;
	}

	const Tile &getTile(int pos) const
	{
		assert(pos >= 0);
		assert(pos < int(tiles.size()));
		return tiles[pos];
	}

	size_t getTileCount() const
	{
		return tiles.size();
	}

private:
	std::vector<Tile> tiles;
};
