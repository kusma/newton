// hscroll_tiler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
// #include "wrap_cimage.h"
#include "image.h"
#include "tileset.h"
#include "tilemap.h"
#include "pixelcache.h"

void error(const char *str)
{
	printf("*** ERROR: %s\n", str);
	exit(1);
}

void print_tiles(const TileSet &ts)
{
	for (int i = 0; i < int(ts.getTileCount()); ++i)
	{
		Tile t = ts.getTile(i);
		for (int y = 0; y < 8; ++y)
		{
			if (y == 0) printf("{ ");
			else printf("  ");

			for (int x = 0; x < 8; ++x)
			{
				printf("%02x ", t.data[y][x]);
			}
			if (y == 7) printf(" }\n");
			else        printf("\n");
		}
	}
}

void dump_tiles(FILE *fp, const TileSet &ts, int bpp)
{
	assert(NULL != fp);
	assert((bpp == 4) || (bpp == 8));
	
	for (int i = 0; i < int(ts.getTileCount()); ++i)
	{
		Tile t = ts.getTile(i);

		if (bpp == 4)
		{
			for (int y = 0; y < 8; ++y)
			{
				for (int x = 0; x < 8; x += 2)
				{
					/* compose byte (is this flipped?) */
					unsigned char byte = t.data[y][x] & 0xF;
					byte |= (t.data[y][x + 1] & 0xF) << 4;

					fwrite(&byte, 1, 1, fp);
				}
			}
		}
		else
		{
			fwrite(t.data, 1, sizeof(t.data), fp);
		}
	}
}

Tile get_tile(const Image &img, int left, int top)
{
	Tile ret;

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			int sx = left + x;
			int sy = top + y;
			ret.data[y][x] = img.getPixelIndex(sx, sy);
//			printf("%d %d - %d\n", sx, sy, ret.data[y][x]);
		}
	}
	
	return ret;
}

struct tile_delta {
	int x, y;
	int tile;
};

#include <SDL.h>

int _tmain(int argc, _TCHAR* argv[])
{
	char *filename = "input.png";

	printf("loading %s...\n", filename);

	RGBQUAD master_pal[256];

	/* load all layers */
	std::vector<Image> layers;
	for (int layer = 0;; ++layer)
	{
		char temp[256];
		sprintf(temp, "in_layer%d.bmp", layer);
		Image img;
		printf("trying to load %s\n", temp);
		if (false == img.load(temp)) break;

		if (img.getColorsUsed() > 16) error("more than 16 colors used in layer");

		for (int c = 0; c < 16; ++c)
		{
			int index = (layer * 16) + c; 
			assert(index >= 0 && index < 256);
			master_pal[index] = img.getPalette()[c];
//			if (0 == layer) master_pal[index].rgbBlue = 0;
		}

//		printf("color used: %d\n", img.getColorsUsed());
		layers.push_back(img);
/*		FIBITMAP *dib = FreeImage_ConvertTo32Bits(img.GetFIBitmap());
		layers.push_back(Image(dib)); */
	}
	if (layers.size() < 1)
	{
		printf("no layers.\n");
		return 1;
	}
#if 0
	std::vector<Image> layers;
	{
		Image img;
		if (false == img.load(filename)) error("failed to load image!");
		printf("image loaded, %dx%d, bpp: %d\n", img.getWidth(), img.getHeight(), img.getBPP());
		layers.push_back(img);
	}
#endif

	std::vector<TileSet> tilesets;
	std::vector<std::vector<int> > maps;
	{

		for (int i = 0; i < int(layers.size()); ++i)
		{
			printf("tiling image %d\n", i);
			Image &img = layers[i];
			if (FIC_PALETTE != FreeImage_GetColorType(img.GetFIBitmap())) error("paletted image required!");
			if ((img.getWidth()  & 7) != 0) error("image width not a multiple of 8");
			if ((img.getHeight() & 7) != 0) error("image height not a multiple of 8");

			printf("generating tileset...\n");

			// create background tile
			Tile bg;
			memset(bg.data, 0, sizeof(bg.data));

			TileSet ts;
			int bg_tile = ts.addTile(bg);
			assert(bg_tile == 0);
			
			std::vector<int> map;
			
			// iterate over x first, so we get our image split into x-segments.
			for (int x = 0; x < img.getWidth(); x += 8)
			{
				for (int y = 0; y < img.getHeight(); y += 8)
				{
					Tile t = get_tile(img, x, y);
					int tile = bg_tile;
					if (memcmp(bg.data, t.data, sizeof(t.data)) != 0)
					{
						tile = ts.addTile(t);
					}
					map.push_back(tile);
				}
			}

			tilesets.push_back(ts);
			maps.push_back(map);

			printf("done generating tileset (%d tile entries, %d map entries)\n", ts.getTileCount(), map.size());
		//	print_tiles(ts);

			{
				printf("dumping tileset...\n");

				char temp[256];
				sprintf(temp, "scroll2_%d.tileset.raw", i);
				FILE *fp = fopen(temp, "wb");

				if (!fp) error("failed to open output file!");
				dump_tiles(fp, ts, 4);
				fclose(fp);
			}

			{
				printf("dumping map...\n");

				char temp[256];
				sprintf(temp, "scroll2_%d.map", i);
				FILE *fp = fopen(temp, "wb");

				if (!fp) error("failed to open output file!");
				for (int i = 0; i < int(map.size()); ++i)
				{
					fwrite(&map[i], sizeof(int), 1, fp);
				}
				fclose(fp);
			}

		}
	}

	{
		printf("dumping master pal\n");
		const RGBQUAD *const pal = master_pal;

		char temp[256];
		sprintf(temp, "scroll2.pal");
		FILE *fp = fopen(temp, "wb");

		if (!fp) error("failed to open output file!");
		for (unsigned int i = 0; i < 256; ++i)
		{
			int r = pal[i].rgbRed   >> (8 - 5);
			int g = pal[i].rgbGreen >> (8 - 5);
			int b = pal[i].rgbBlue  >> (8 - 5);
			unsigned short col = b | (g << 5) | (r << 10);
			fwrite(&col, 2, 1, fp);
		}
		fclose(fp);
	}

    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return 1;
	SDL_Surface	*screen = SDL_SetVideoMode(240, 160, 8, SDL_DOUBLEBUF);
	
	/* create pixel caches */
	std::vector<PixelCache> pixel_caches;
	for (size_t i = 0; i < layers.size(); ++i)
	{
		pixel_caches.push_back(PixelCache(layers[i]));
	}

	SDL_Color colors[256];
	for (int i = 0; i < 256; ++i)
	{
		colors[i].r = 0;
		colors[i].g = 255;
		colors[i].b = 0;

		assert(layers.size() > 0);
		colors[i].r = master_pal[i].rgbRed;
		colors[i].g = master_pal[i].rgbGreen;
		colors[i].b = master_pal[i].rgbBlue;
	}
	SDL_SetPalette(screen, SDL_PHYSPAL, colors, 0, 256);

	std::vector<std::vector<TileMap> > frames;
	{
		/* simulate scroller */
		bool done = false;
		while (false == done)
		{
			static int frame = 0;
			printf("frame %d\n", frame++);

			if(SDL_MUSTLOCK(screen)) 
			{
				SDL_LockSurface(screen);
			}

			std::vector<TileMap> optimized_maps;
			for (int i = 0; i < int(layers.size()); ++i)
			{
				optimized_maps.push_back(
					TileMap(
						layers[i].getWidth() / 8,
						layers[i].getHeight() / 8
						)
					);
			}
			
			for (int y = 0; y < screen->h; y++)
			{
				unsigned char *scanline = (unsigned char*)screen->pixels + y * screen->pitch;
				for (int x = 0; x < screen->w; x++)
				{
					int color = 0;
					for (size_t i = 0; i < pixel_caches.size(); ++i)
					{
						int scroll = frame / (1 + int(i));

						int layer_x = x + scroll;
						int layer_y = y;

						int tile_x = layer_x / 8;
						int tile_y = layer_y / 8;
						int tile = tile_y + tile_x * (layers[i].getHeight() / 8);

						color = pixel_caches[i].getPixelIndex(layer_x, layer_y);

						/* lookup color in tileset */
						assert(maps.size() > i);
						assert(int(maps[i].size()) > tile);
						if (tile >= int(maps[i].size()))
						{
							done = true;
							break;
						}
						int real_tile = maps[i][tile];

						const Tile &t = tilesets[i].getTile(real_tile);
						int color2 = t.data[layer_y & 7][layer_x & 7];
						assert(color == color2);

						if (0 != color)
						{
							color += int(i) * 16;
							// huston, we have a visible pixel. make sure optimized map knows.
							optimized_maps[i].setEntry(tile_x, tile_y, real_tile);
							break;
						}
					}
					scanline[x] = color;
				}
			}

			/* count tiles */
			int tile_count = 1;
			for (int i = 0; i < int(optimized_maps.size()); ++i)
			{
				const TileMap &tm = optimized_maps[i];
				for (int y = 0; y < tm.getHeight(); ++y)
				{
					for (int x = 0; x < tm.getWidth(); ++x)
					{
						int entry = tm.getEntry(x, y);
						if (0 != entry) tile_count++;
					}
				}
			}
			printf("tiles: %d\n", tile_count);

			frames.push_back(optimized_maps);

			if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);

			SDL_Flip(screen);

			SDL_Event	event;
			while (SDL_PollEvent(&event))
			{
				switch (event.type) 
				{
					case SDL_QUIT:
						done = true;
					break;

					case SDL_KEYDOWN:
						if (event.key.keysym.sym == SDLK_ESCAPE) done = true;
					break;
				}
			}
		}
	}

	for (size_t i = 0; i < pixel_caches.size(); ++i)
	{
		char temp[256];
		sprintf(temp, "out_layer%d.png", i);
		pixel_caches[i].getImage().save(temp);
	}


	/* make tile deltas */
	std::vector<std::vector<std::vector<tile_delta> > > tile_deltas;
	for (int i = 0; i < int(frames.size()); ++i)
	{
		std::vector<TileMap> &frame = frames[i];
		std::vector<std::vector<tile_delta> > frame_tile_deltas;

		for (int j = 0; j < int(frame.size()); ++j)
		{
			int scroll = j / (1 + i);

			TileMap &tm = frame[j];
			std::vector<tile_delta> layer_tile_deltas;
			if (i < 1)
			{
				/* insert all tiles */
				for (int y = 0; y < tm.getHeight(); ++y)
				{
					for (int x = (scroll / 8); x < (scroll / 8) + (240 / 8); ++x)
					{
//						printf("%d ", tm.getEntry(x, y));
						tile_delta td;
						td.x = x;
						td.y = y;
						td.tile = tm.getEntry(x, y);

						layer_tile_deltas.push_back(td);
					}
				}
			}
			else
			{
				assert(i > 0);
				assert(i < frames.size());
				std::vector<TileMap> &old_frame = frames[i-1];
				
				assert(frame.size() == old_frame.size());
				TileMap &old_tm = old_frame[j];

				/* make corrections */
				for (int y = 0; y < tm.getHeight(); ++y)
				{
					for (int x = 0; x < tm.getWidth(); ++x)
//					for (int x = 0; x < (240 / 8) + 8; ++x)
//					for (int x = (scroll / 8); x < (scroll / 8) + (240 / 8); ++x)
					{
//						printf("%d ", tm.getEntry(x, y));
						if (tm.getEntry(x, y) != old_tm.getEntry(x, y))
						{
							tile_delta td;
							td.x = x;
							td.y = y;
							td.tile = tm.getEntry(x, y);

							layer_tile_deltas.push_back(td);
						}
					}
				}
			}
			printf("layer deltas: %d\n", layer_tile_deltas.size());
			frame_tile_deltas.push_back(layer_tile_deltas);
		}
		tile_deltas.push_back(frame_tile_deltas);
	}


	FILE *fp_offsets = fopen("scroll2_tile_delta_offsets.bin", "wb");
	if (!fp_offsets) error("mongohore");

	FILE *fp_deltas = fopen("scroll2_tile_deltas.bin", "wb");
	if (!fp_deltas) error("mongohore");

	for (int i = 0; i < int(tile_deltas.size()); ++i)
	{
		std::vector<std::vector<tile_delta> > &frame_deltas = tile_deltas[i];
		for (int j = 0; j < int(frame_deltas.size()); ++j)
		{
			std::vector<tile_delta> &layer_deltas = frame_deltas[j];
	
			unsigned short offset = int(layer_deltas.size());
			assert(offset == int(layer_deltas.size()));
			fwrite(&offset,    sizeof(offset), 1, fp_offsets);

			for (int k = 0; k < int(layer_deltas.size()); ++k)
			{
				tile_delta td = layer_deltas[k];
				unsigned short x    = td.x;
				assert(x == td.x);
				unsigned short y    = td.y;
				assert(y == td.y);
				unsigned short tile = td.tile;
				assert(tile == td.tile);

				fwrite(&x,    sizeof(x),    1, fp_deltas);
				fwrite(&y,    sizeof(y),    1, fp_deltas);
				fwrite(&tile, sizeof(tile), 1, fp_deltas);
			}
		}
	}

	{
		/* write terminating offset */
		unsigned short offset = 0xFFFF;
		fwrite(&offset, sizeof(offset), 1, fp_offsets);
	}

	fclose(fp_deltas);
	fclose(fp_offsets);

/*
	std::vector<std::vector<std::vector<tile_delta> > > tile_deltas;
*/
	SDL_Quit();

	return 0;
}
