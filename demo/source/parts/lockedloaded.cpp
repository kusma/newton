#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_sprites.h>
#include "../fb.h"
#include "../base.h"
#include "../vcount.h"
#include "../part.h"
#include "../timer.h"


extern u16 lockedloaded_pal[];
extern u8  lockedloaded_tex[];


static int boost=40;

static int blanks = 0;

static void vblank()
{
	blanks++;
}

static void init()
{
	VBlankIntrWait();
	CpuFastSet(lockedloaded_pal, BG_COLORS, 512 / 4);
	fb::setMode(MODE_4 | BG2_ON);
}

namespace parts {
	void lockedloaded(fixed16 start, fixed16 end)
	{
		part::set_part(init, vblank);

		int scrollx=0;
		int swaps=0;
		int img_width = 2560 / 2; //356/2;
		while (timer::getBeat() < end)
		{

			const u16* src_linep = (const u16*)lockedloaded_tex;
			u16* dst_linep = (u16*)fb::bb;
			for (int y=0; y<160; y++, src_linep+=img_width, dst_linep+=240/2)
			{
				CpuFastSet(src_linep+(scrollx>>1), dst_linep, COPY16|(240>>1));			
			}
			
			VBlankIntrWait();
			part::swap();
	
			scrollx += 16;
			swaps++;
		}
	}
};
