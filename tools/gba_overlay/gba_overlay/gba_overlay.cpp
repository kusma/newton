// gba_overlay.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

extern "C" {
#include "tinyptc.h"
}

#pragma pack(push, 1)
#include "c:/gba/devkitPro/libgba/include/gba_sprites.h"
#pragma pack(pop)

#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))

#define VRAM_TOTAL_SIZE (0x6018000 - 0x6000000)
#define VRAM_OBJ_SIZE   (0x6018000 - 0x6014000)

#include "tilefield.h"
TileField tilefield;

/* Tile tilefield[TILEFIELD_HEIGHT][TILEFIELD_WIDTH]; */
bool low_color = false;

void highlight_tile(unsigned *dst, int topleft_x, int topleft_y)
{
	int min_x = max(0, topleft_x);
	int min_y = max(0, topleft_y);

	int max_x = min(SCREEN_WIDTH,  topleft_x + TILE_WIDTH);
	int max_y = min(SCREEN_HEIGHT, topleft_y + TILE_HEIGHT);

	unsigned iy = 0;
	for (int y = min_y; y < max_y; ++y, ++iy)
	{
		unsigned ix = 0;
		for (int x = min_x; x < max_x; ++x, ++ix)
		{
			unsigned color = dst[x + y * SCREEN_WIDTH];
			dst[x + y * SCREEN_WIDTH] = ~color;
		}
	}
}

#include "image.h"

extern "C" int da_zoom;

unsigned screen[SCREEN_WIDTH * SCREEN_HEIGHT];


void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
	printf("\n*** ");
	printf("%s Format\n", FreeImage_GetFormatFromFIF(fif));
	printf(message);
	printf(" ***\n");
}

void export_sprites()
{
	printf("EKS PORT\n");

	char filename[_MAX_FNAME + 1];
	filename[0] = '\0';

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = filename;
	ofn.nMaxFile = _MAX_FNAME;
	ofn.lpstrDefExt = "h";
	ofn.lpstrFilter = "OAM File (*.oam)\0*.oam\0All Files (*.*)\0*.*\0\0";
	ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT; 

	if (!GetSaveFileName(&ofn)) return;
	printf("filename: %s\n", ofn.lpstrFile);

	char drive[_MAX_FNAME], dir[_MAX_FNAME], fname[_MAX_FNAME], ext[_MAX_FNAME];
	char ss_filename[_MAX_FNAME], tileset_filename[_MAX_FNAME];
	_splitpath(ofn.lpstrFile, drive, dir, fname, ext);
	_makepath(ss_filename, drive, dir, fname, "ss.png");
	_makepath(tileset_filename, drive, dir, fname, "tileset.png");

	char tileset_raw_filename[_MAX_FNAME];
	char tileset_pal_filename[_MAX_FNAME];
	_makepath(tileset_raw_filename, drive, dir, fname, "tileset.raw");
	_makepath(tileset_pal_filename, drive, dir, fname, "tileset.pal");


	{
		CImage img;
		img.Create(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
		unsigned *data = (unsigned *)img.GetBits();
		
		for (unsigned y = 0; y < TILEFIELD_HEIGHT; ++y)
		{
			for (unsigned x = 0; x < TILEFIELD_WIDTH; ++x)
			{
				tilefield(x, y).draw(data, img.GetPitch());
			}
		}
		
		img.Save(ss_filename);
	}
	
	CImage img;
	
	printf("cre8te tilesetshit %i %i\n", 8, TILEFIELD_WIDTH * TILEFIELD_HEIGHT * (TILE_WIDTH / 8) * TILE_HEIGHT);
	printf("cre8te tilesetshit (%i %i)\n", 8 / 8, (TILEFIELD_WIDTH * TILEFIELD_HEIGHT * (TILE_WIDTH / 8) * TILE_HEIGHT) / 8);
	
	FILE *fp = fopen(filename, "wb");
	if (!fp)
	{
		MessageBox(0, "failed to write file", 0, MB_OK);
		return;
	}

//	if (!img.Create(8, TILEFIELD_WIDTH * TILEFIELD_HEIGHT * (TILE_WIDTH / 8) * TILE_HEIGHT, 32))
	if (!img.Create(8, TILEFIELD_WIDTH * TILEFIELD_HEIGHT * (TILE_WIDTH / 8) * TILE_HEIGHT, 32))
	{
		printf("failed to create tileset-image.. :( %i %i\n", 8, 8 * (TILEFIELD_WIDTH / 8) * (TILEFIELD_HEIGHT / 8));
	}
	
	unsigned *data = (unsigned *)img.GetBits();

	OBJATTR objattrs[TILEFIELD_WIDTH * TILEFIELD_HEIGHT];
	int tile = 0;
	for (unsigned y = 0; y < TILEFIELD_HEIGHT; ++y)
	{
		for (unsigned x = 0; x < TILEFIELD_WIDTH; ++x)
		{
			const Tile &t = tilefield(x, y);
			OBJATTR &o = objattrs[x + y * TILEFIELD_WIDTH];
			
			if (t.isEnabled())
			{
				o.attr0 = OBJ_Y(t.getPosY()) | OBJ_MODE(1);
				o.attr0 |= (low_color == true) ? (OBJ_16_COLOR) : (OBJ_256_COLOR);
				o.attr1 = OBJ_X(t.getPosX());
				
				/* change these settings if you change the size of the sprites */
				o.attr0 |= OBJ_SHAPE(2);
//				o.attr1 |= OBJ_SIZE(2);
				
				if (t.getScaleX() != 0 || t.getScaleY() != 0)
				{
					o.attr0 |= OBJ_ROT_SCALE_ON | OBJ_DOUBLE;
					o.attr1 |= OBJ_SIZE(0);
				}
				else
				{
					o.attr1 |= OBJ_SIZE(2);
				}
				
				t.plotTiles(data, tile, img.GetPitch());
				
				o.attr2 = 512 + tile * (low_color ? 1 : 2);
				tile += ((TILE_WIDTH >> t.getScaleX()) / 8) * ((TILE_HEIGHT >> t.getScaleY()) / 8);
			}
			else
			{
				o.attr0 = OBJ_DISABLE;
				o.attr1 = 0;
				o.attr2 = 0;
			}
		}
	}

	FIBITMAP *dib2 = FreeImage_ConvertTo24Bits(img.GetFIBitmap());
	FIBITMAP *dib3 = FreeImage_Copy(dib2, 0, TILEFIELD_WIDTH * TILEFIELD_HEIGHT * (TILE_WIDTH / 8) * TILE_HEIGHT, 8, TILEFIELD_WIDTH * TILEFIELD_HEIGHT * (TILE_WIDTH / 8) * TILE_HEIGHT - (TILE_WIDTH / 8) * (TILE_HEIGHT / 8) * tile);
	FreeImage_Unload(dib2);
	dib2 = dib3;

	/*
	dib3 = FreeImage_Dither(dib2, FID_BAYER4x4);
	if (NULL == dib3)
	{
		printf("WTF 1?!");
	}
	FreeImage_Unload(dib2);
	dib2 = dib3;
*/

	RGBQUAD forced_pal[] = { {0, 255, 0} };
	dib3 = FreeImage_ColorQuantizeEx(dib2, FIQ_WUQUANT, low_color ? 16 : 256, ARRAY_SIZE(forced_pal), forced_pal);
	if (NULL == dib3)
	{
		printf("WTF 2?!");
	}
	FreeImage_SetBackgroundColor(dib3, &forced_pal[0]);

//	FIBITMAP *dib3 = FreeImage_ColorQuantizeEx(dib2);
	FreeImage_Unload(dib2);
	dib2 = dib3;

	if (dib2 != NULL)
	{
		RGBQUAD *pal = FreeImage_GetPalette(dib2);
		int green = 0;
		for (int i = 0; i < 256; ++i)
		{
			if ((pal[i].rgbRed == 0) && (pal[i].rgbGreen == 255) && (pal[i].rgbBlue == 0))
			{
				green = i;
				break;
			}
		}
		printf("green: %d\n", green);

		{
			FILE *fp = fopen(tileset_raw_filename, "wb");
			if (low_color)
			{
				for (int y = 0; y < (TILE_WIDTH / 8) * (TILE_HEIGHT / 8) * tile; ++y )
				{
					for (int x = 0; x < 8 / 2; ++x )
					{
						BYTE idx1, idx2;
						FreeImage_GetPixelIndex(dib2, x * 2 + 0, y, &idx1);
						if (idx1 == 0) idx1 = green;
						else if(idx1 == green) idx1 = 0;

						FreeImage_GetPixelIndex(dib2, x * 2 + 1, y, &idx2);
						if (idx2 == 0) idx2 = green;
						else if(idx2 == green) idx2 = 0;

						u8 color = idx1 | (idx2 << 4);
						fwrite(&color, 1, 1, fp);
					}
				}
			}
			else
			{
				for (int y = 0; y < (TILE_WIDTH / 8) * (TILE_HEIGHT / 8) * tile; ++y )
				{
					for (int x = 0; x < 8; ++x )
					{
						BYTE idx;
						FreeImage_GetPixelIndex(dib2, x, y, &idx);
						if (idx == 0) idx = green;
						else if(idx == green) idx = 0;
						fwrite(&idx, 1, 1, fp);
					}
				}
			}

			fclose(fp);

		}
		{
			FILE *fp = fopen(tileset_pal_filename, "wb");
			for (int i = 0; i < 256; ++i)
			{
				int idx = i;
				if (idx == 0) idx = green;
				else if (idx == green) idx = 0;

				u16 col = 
					((pal[idx].rgbRed   >> (8 - 5)) << 10) |
					((pal[idx].rgbGreen >> (8 - 5)) <<  5) |
					((pal[idx].rgbBlue  >> (8 - 5)) <<  0);
				fwrite(&col, 2, 1, fp);
			}
			fclose(fp);
		}
	}
	else
	{
		printf("eek! failed!!\n");
	}
	FreeImage_Unload(dib2);
	
/*
	for (unsigned y = 0; y < TILEFIELD_HEIGHT; ++y)
	{
		for (unsigned x = 0; x < TILEFIELD_WIDTH; ++x)
		{
			tilefield[y][x].draw(screen, WIDTH * 4);
		}
	}
*/
	((OBJAFFINE*)objattrs)[0].pa = 1 << 7;
	((OBJAFFINE*)objattrs)[0].pb = 0;
	((OBJAFFINE*)objattrs)[0].pc = 0;
	((OBJAFFINE*)objattrs)[0].pd = 1 << 7;
	
	fwrite(objattrs, sizeof(OBJATTR), TILEFIELD_WIDTH * TILEFIELD_HEIGHT, fp);
	img.Save(tileset_filename);
	fclose(fp);
}

bool last_keys[256] = { false };
bool keydown(unsigned k)
{
	bool key = GetAsyncKeyState(k) != 0;
	bool change = false;
	if (last_keys[k] != key && key) change = true;
	last_keys[k] = key;
	return change;
}


int main(int argc, char* argv[])
{
	memset(screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT * 4);

	printf("%d\n", argc);
	if (argc != 2)
	{
		MessageBox(0, "no image to load", 0, MB_OK);
		exit(1);
	}

	FreeImage_SetOutputMessage(FreeImageErrorHandler);

/*
	unsigned w, h;
	unsigned *image;
	if (!image_load(argv[1], &image, &w, &h))
*/
	CImage image;
	if (!image.Load(argv[1]))
	{
		printf("error: failed to load image\n");
		exit(1);
	}

	if (image.GetWidth() != SCREEN_WIDTH || image.GetHeight() != SCREEN_HEIGHT)
	{
		printf("error: image not right size (expected %i %i)\n", SCREEN_WIDTH, SCREEN_HEIGHT);
		exit(1);
	}

	for (unsigned y = 0; y < TILEFIELD_HEIGHT; ++y)
	{
		for (unsigned x = 0; x < TILEFIELD_WIDTH; ++x)
		{
//			tilefield(x, y) = Tile(image, x * TILE_WIDTH, y * TILE_HEIGHT, 1, 1);
			tilefield(x, y) = Tile(image.GetBits(), image.GetPitch(), x * TILE_WIDTH, y * TILE_HEIGHT, 1, 1);
		}
	}
	int memory_used = (TILE_WIDTH / 2) * (TILE_HEIGHT / 2) * TILEFIELD_WIDTH * TILEFIELD_HEIGHT;


	printf("%i\n", (SCREEN_WIDTH / 8) * (SCREEN_HEIGHT / 8));
	printf("%i\n", VRAM_OBJ_SIZE / (8 * 8));
	printf("%i %i\n", VRAM_OBJ_SIZE, (8 * 4800));
	printf("%i\n", (16 * 1024) / (8 * 8));

	printf("%i\n", 954 / (26 / 8));

	if (!ptc_open("test", SCREEN_WIDTH, SCREEN_HEIGHT)) return 1;
	HWND win = GetForegroundWindow();

	printf("memory used: %i\n", memory_used);
	while (1)
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(win, &p);
		p.x /= da_zoom;
		p.y /= da_zoom;
		
		for (int y = 0; y < SCREEN_HEIGHT; ++y)
		{
			for (int x = 0; x < SCREEN_WIDTH; ++x)
			{
				screen[y * SCREEN_WIDTH + x] = 0x00FF00;
			}
		}
		/* update display */
		for (unsigned y = 0; y < TILEFIELD_HEIGHT; ++y)
		{
			for (unsigned x = 0; x < TILEFIELD_WIDTH; ++x)
			{
				tilefield(x, y).draw(screen, SCREEN_WIDTH * 4);
			}
		}
		
		bool change = (GetForegroundWindow() == win) && keydown(VK_LBUTTON);
		
		if (p.x >= 0 && p.x < SCREEN_WIDTH && p.y >= 0 && p.y < SCREEN_HEIGHT)
		{
			int tx = p.x >> TILE_LOG2_WIDTH;
			int ty = p.y >> TILE_LOG2_HEIGHT;
			
			int tile_pos_x = tx * TILE_WIDTH;
			int tile_pos_y = ty * TILE_HEIGHT;
			
			highlight_tile(screen, tile_pos_x, tile_pos_y);
			if (change)
			{
				Tile &t = tilefield(tx, ty);
				
				// subtract used memory
				if (t.isEnabled()) memory_used -= t.getSize();
				
				Tile new_tile;
				
				if      (t.isEnabled() && t.getScaleX() != 0) new_tile = Tile(image.GetBits(), image.GetPitch(), tile_pos_x, tile_pos_y, 0, 0);
				else if (t.isEnabled() && t.getScaleX() == 0) new_tile = Tile(tile_pos_x, tile_pos_y);
				else                                  new_tile = Tile(image.GetBits(), image.GetPitch(), tile_pos_x, tile_pos_y, 1, 1);
				t = new_tile;
				
				// add memory
				if (t.isEnabled()) memory_used += t.getSize();
				
				/* log */
				printf("memory used:\n", memory_used, VRAM_OBJ_SIZE, VRAM_OBJ_SIZE - memory_used);
				if (low_color) printf("%s 16 color:  %i bytes of %i bytes (%i bytes left)\n", low_color ? "*" : " ", memory_used / 2, VRAM_OBJ_SIZE, VRAM_OBJ_SIZE - (memory_used / 2));
				else           printf("%s 256 color: %i bytes of %i bytes (%i bytes left)\n", low_color ? " " : "*", memory_used, VRAM_OBJ_SIZE, VRAM_OBJ_SIZE - memory_used);
			}
		}

		if (GetForegroundWindow() == win && keydown('D'))
		{
			low_color = !low_color;
			printf("%s color mode\n", low_color ? "16" : "256");
		}

		if (GetForegroundWindow() == win && keydown('E')) export_sprites();
		
		ptc_update(screen);
	}

	return 0;
}

