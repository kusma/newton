#include "grideffects.h"
#include <stdio.h>
#include <stdlib.h>
Vector3 direction[GRID_WIDTH * GRID_HEIGHT] EWRAM_DATA;

void grideffects::init(void)
{
	Vector3 *v = &direction[0];

	for (int y = GRID_HEIGHT; y; --y)
	{
		for (int x = GRID_WIDTH; x; --x)
		{
#if 0
			Vector3 dir((x - SCREEN_WIDTH / 2), (y - SCREEN_HEIGHT / 2), 200);

			dir.x *= SCREEN_WIDTH;
			dir.x /= GRID_WIDTH;

			dir.y *= SCREEN_HEIGHT;
			dir.y /= GRID_HEIGHT;

			dir = dir.normalize();
			printf("%f %f %f\n", FIXED_TO_FLOAT(dir.x), FIXED_TO_FLOAT(dir.y), FIXED_TO_FLOAT(dir.z));
			printf("%f\n", FIXED_TO_FLOAT(dir.x) * FIXED_TO_FLOAT(dir.x) + FIXED_TO_FLOAT(dir.y) * FIXED_TO_FLOAT(dir.y) + FIXED_TO_FLOAT(dir.z) * FIXED_TO_FLOAT(dir.z));
//			printf("%f\n", FIXED_TO_FLOAT(dir.mag()));
			*v++ = dir;
#else
			Vector3 dir(
				fixed16::make_raw(int(x - (GRID_WIDTH / 2))),
				fixed16::make_raw(int(y - (GRID_HEIGHT / 2))),
//				fixed16::make_raw(40)
				fixed16::make_raw(70)
			);

			dir.x *= SCREEN_WIDTH;
			dir.x /= GRID_WIDTH;

			dir.y *= SCREEN_HEIGHT;
			dir.y /= GRID_HEIGHT;

			dir.normalize();

			*v++ = dir;
#endif
		}
	}
//	exit(0);
}
