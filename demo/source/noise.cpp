#include "noise.h"
#include "math.h"

#include <gba_video.h>
#include <gba_sprites.h>

void noise::setup()
{
	// setup tileset
	for (int i = 0; i < 8 * 8 * 128 / 2; ++i)
	{
		((u16*)BITMAP_OBJ_BASE_ADR)[i] = cheap_rand();
	}

	// setup palette
	for (int i = 0; i < 256; ++i)
	{
		u32 r = cheap_rand();
		r &= 31;
		OBJ_COLORS[i] = RGB5(r, r, r);
	}

	noise::update();
}

void noise::update()
{
	for (int y = 0; y < 5; ++y)
	{
		for (int x = 0; x < 15; ++x)
		{
			OBJATTR &o = OAM[x + y * 15];
			u32 r = cheap_rand();
			
			o.attr0 = OBJ_Y(y * 32) | ATTR0_TALL;
			o.attr1 = OBJ_X(x * 16) | OBJ_SIZE(Sprite_16x32);
			o.attr2 = OBJ_CHAR(512 + (r & 127));
		}
	}
}
