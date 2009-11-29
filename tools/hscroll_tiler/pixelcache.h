#pragma once

#include "image.h"

class PixelCache
{
public:
	PixelCache(const Image &img) : src_img(img)
	{
		dst_img.create(img.getWidth(), img.getHeight(), img.getBPP());
		dst_img.setPalette(img.getPalette());
		for (int y = 0; y < dst_img.getHeight(); ++y)
		{
			for (int x = 0; x < dst_img.getWidth(); ++x)
			{
				dst_img.setPixelIndex(x, y, 0);
			}
		}
	}

	PixelCache(const PixelCache &pc) : src_img(pc.src_img), dst_img(pc.dst_img)
	{

	}

	RGBQUAD getPixelColor(int x, int y) const
	{
		RGBQUAD color = src_img.getPixelColor(x, y);
		bool err = dst_img.setPixelColor(x, y, color);
		assert(err == true);
		return color;
	}

	BYTE getPixelIndex(int x, int y) const
	{
		BYTE color_index = src_img.getPixelIndex(x, y);

		Tile t;
		unsigned int tile = (x / 8) * (src_img.getHeight() / 8) + y / 8;
		if (tile < ts.getTileCount())
		{
			t = ts.getTile(tile);
		}

		bool err = dst_img.setPixelIndex(x, y, color_index);
		assert(err == true);

		return color_index;
	}

	const Image &getImage() const
	{
		return dst_img;
	}

private:
	TileSet ts;
	Image src_img;
	mutable Image dst_img;
};
