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


extern const u16 skate0_pal[];
extern const u16 skate0_flikk[];

extern const u16 skate1_pal[];
extern const u16 skate1_flikk[];

extern const u16 faceplant_pal[];
extern const u16 faceplant_flikk[];



static const u16* pal;

namespace parts{


	static void init(void) {
		VBlankIntrWait();
		CpuFastSet(pal, BG_COLORS, 512 / 4);
		//FORI(256) BG_COLORS[i] = RGB8(i*8,i*8,i*8);
		// the matrix
		REG_BG2PA = 1<<7;
		REG_BG2PB = 0;
		REG_BG2PC = 0;
		REG_BG2PD = -1<<7;
		REG_BG2Y = (80<<8)-1;
		REG_BG2X = 0;
		fb::setMode(MODE_4 | BG2_ON | OBJ_1D_MAP);
	}
	
	static int blanks=0;
	static void vblank() {
		blanks++;
	}

	static void hei(const u16* pal, const u16* flikkData, fixed16 end, int flikkIndex, int offset=0) {
		Flikk flikk(flikkData);
		::pal=pal;
		part::set_part(init, vblank);
		int speed=0;
		if (flikkIndex==0 || flikkIndex==2) speed=0;
		else if (flikkIndex==1) speed=2;

		blanks=0;
		while(timer::getBeat() < end) {
			flikk.drawMov(fb::bb);
			flikk.setFrame((blanks>>speed)+offset);
			VBlankIntrWait();
			if (flikkIndex==1) {
				// fadeout
				fixed16 fader = (end - timer::getBeat())*(1.0/4);
				if (fader<fixed16(1.0)) {
					int amt = fader.get_val()>>8;
					palette::boost(BG_COLORS, pal, 256, amt-256);
				}
				
			}
			part::swap();
		}
	}

	void skate(fixed16 start, fixed16 end, int flikkIndex, int offset) {
		// fix the matrix
		REG_BG2PA = 1<<8;
		REG_BG2PB = 0;
		REG_BG2PC = 0;
		REG_BG2PD = 1<<8;
		REG_BG2Y = 0;

		// MUST do this before set_part!
		if (flikkIndex==0) {
			hei(skate0_pal, skate0_flikk, end, flikkIndex, offset);
		} else if (flikkIndex==1) { 
			hei(faceplant_pal, faceplant_flikk, end, flikkIndex);
		} else if (flikkIndex==2) { 
			hei(skate1_pal, skate1_flikk, end, flikkIndex, offset);
		}


		// fix the matrix
		REG_BG2PA = 1<<8;
		REG_BG2PB = 0;
		REG_BG2PC = 0;
		REG_BG2PD = 1<<8;
		REG_BG2Y = 0;
	}

};
