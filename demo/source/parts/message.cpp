#include "../exstructs.h"

#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <stdio.h>
#include "../part.h"
#include "../parts.h"

#include "../flikk.h"
#include "../fb.h"
#include "../palette.h"
#include "../3dthen.h"
#include "../timer.h"
#include <gba_video.h>


extern const u8 sfc_bp_end_raw[];
extern const u8 sfc_bp_wait4part2_raw[];

namespace parts{


	static void init(void)
	{
		VBlankIntrWait();
		
		// fix the matrix
		REG_BG2PA = 1<<8;
		REG_BG2PB = 0;
		REG_BG2PC = 0;
		REG_BG2PD = 1<<8;
		REG_BG2Y = 0;
		
		fb::setMode(MODE_4 | BG2_ON);
		
		int svart=0;
		CpuFastSet(&svart, BG_COLORS, FILL|(512/4));
	}
	
	int fade=0;
	static void vblank() { }


	void message(fixed16 start, fixed16 end, int index)
	{
		part::set_part(init, vblank);

		const u8* bilde;
		if (index==0) bilde = sfc_bp_end_raw;
		else          bilde = sfc_bp_wait4part2_raw;
		
		CpuFastSet(bilde, fb::bb, (240*160/4));
		part::swap();

		static int amt=0;
		while(timer::getBeat() < end)
		{
			// fadeout
			fixed16 fader = (end - timer::getBeat())*(1.0/4);
			if (fader<fixed16(1.0)) {
				amt = fader.get_val()>>8;
			} else {
				amt+=2;
			}

			if (amt>255) amt=255;
			if (amt<0)   amt=0;
			
			VBlankIntrWait();
			FORI(256) {
				int v = (i * amt) >> 8;
				BG_COLORS[i] = RGB8(v,v,v);
			}
		}


	}

};
