#include "exstructs.h"

#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_sprites.h>
#include <pimpmobile.h>
#include "fb.h"
#include "config.h"
#include "vcount.h"
#include "palette.h"
#include "glow.h"
#include "3dthen.h"
//#include "rle.h"
//#include "events.h"

extern u16 shit_pal[];
extern u8  shit_raw[];

extern u16 faced_pal[];
extern u8  faced_raw[];

extern u16 clowns_pal[];
extern u8  clowns_raw[];

namespace parts {
	void end()
	{
		CpuFastSet(shit_pal, BG_COLORS, 512 / 4);
		CpuFastSet(shit_raw, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
		fb::swap();

		while (pimp_get_row() < 28);

		CpuFastSet(faced_pal, BG_COLORS, 512 / 4);
		CpuFastSet(faced_raw, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
		fb::swap();

		while (pimp_get_order() == 0x16);

		CpuFastSet(clowns_pal, BG_COLORS, 512 / 4);
		CpuFastSet(clowns_raw, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
		fb::swap();

		while (pimp_get_row() < 20);
		pimp_close();
	}
};
