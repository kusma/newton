#pragma once

class Tile
{
public:

	bool operator==(const Tile &t) const
	{
		return memcmp(t.data, data, sizeof(data)) == 0 ? true : false;
	}

	unsigned char data[8][8];
};
