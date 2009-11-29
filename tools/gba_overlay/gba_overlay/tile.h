#pragma once

#define TILE_LOG2_WIDTH 4
#define TILE_LOG2_HEIGHT 5
#define TILE_WIDTH (1 << TILE_LOG2_WIDTH)
#define TILE_HEIGHT (1 << TILE_LOG2_HEIGHT)

class Tile
{
public:
	Tile(unsigned topleft_x = 0, unsigned topleft_y = 0);
	Tile(const void *src, int src_pitch, int topleft_x, int topleft_y, int scale_x, int scale_y);
	
	void draw(unsigned *dst, int pitch) const;
	void plotTiles(unsigned *dst, int tile_offset, int pitch) const;
	
	int getSize()
	{
		return (TILE_WIDTH >> scale_x) * (TILE_HEIGHT >> scale_y);
	}

	bool isEnabled() const { return enabled; }
	int getPosX() const { return topleft_x; }
	int getPosY() const { return topleft_y; }
	int getScaleX() const { return scale_x; }
	int getScaleY() const { return scale_y; }

private:
	bool enabled;
	unsigned data[TILE_WIDTH * TILE_HEIGHT];
	unsigned scale_x, scale_y;
	unsigned topleft_x, topleft_y;
};
