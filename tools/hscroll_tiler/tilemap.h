#pragma once

class TileMap
{
public:
	TileMap(int w, int h)
		: w(w), h(h)
	{
		data.resize(w * h);
		clear(0);
	}

	void clear(int value)
	{
		for (int i = 0; i < w * h; ++i)
		{
			data[i] = value;
		}
	}

	void setEntry(int x, int y, int entry)
	{
		assert(x >= 0);
		assert(x < w);
		assert(y >= 0);
		assert(y < h);
		data[x + y * w] = entry;
	}

	int getEntry(int x, int y) const
	{
		assert(x >= 0);
		assert(x < w);
		assert(y >= 0);
		assert(y < h);
		return data[x + y * w];
	}

	int getWidth() const
	{
		return w;
	}

	int getHeight() const
	{
		return h;
	}

private:
	int w, h;
	std::vector<int> data;
};
