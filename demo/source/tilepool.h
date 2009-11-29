#ifndef TILEPOOL_H
#define TILEPOOL_H

#include "base.h"

struct tile
{
	int index;
	struct tile *prev;
	struct tile *next;
};

#define MAX_TILES (1024 - (32 * 2))

class TilePool
{	
public:
	TilePool() : first_taken(NULL), first_free(NULL)
	{
		reset();
	}
	
	void reset();
	int getTile();
	void freeTile(int tile);
	
private:
	struct tile  tiles[MAX_TILES];
	struct tile *first_free;
	struct tile *first_taken;
};

#endif // TILEPOOL_H
