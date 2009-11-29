#include "exstructs.h"

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
#include "fb.h"
#include "config.h"
#include "vcount.h"
#include "palette.h"
#include "3dthen.h"
#include "decomp.h"
#include "flikk.h"
#include "flikkfx.h"
#include "glow.h"
#include "exposure.h"
#include <pimpmobile.h>


extern const u16 favetxt_flikk[];
extern const u16 hdr_wall_xpal[];
extern const u8 wall_raw[];
extern const u8 hdrbump_ebump[];
extern const u8 kulenm_ebump[];
extern const u8 alleyenv_tex[];
extern const u8 alleyenvsb_tex[];
extern const u8 normalmap_ebump[];


static int blanks = 0;
void global_vblank();
static void vblank()
{
	pimp_vblank();
	glow::vblank(); 
	blanks++;
	pimp_frame();
	global_vblank();
}

static volatile int frameno = -1;
static void callback(int type, int data)
{
	if (data==1) {
		frameno++;
	}
}



//static u16 sxpal[256*4] EWRAM_DATA;

extern const u16 glowpal_yellow[];


namespace parts
{
	void hdr_wall(int len)
	{
		irqSet(IRQ_VBLANK, vblank);
		irqEnable(IRQ_VBLANK);
		
		
		Flikk flikk(favetxt_flikk);
		
//		glow::init(NULL);
		int img_width = 2076/4;
		int scrollx = 0;
		int swaps=0; 
		
		pimp_set_callback(callback);

		int startfade = 0x10;
		int blinkdown = 0;
		
		int start_order = pimp_get_order();
		while ((pimp_get_order() - start_order < len) && (pimp_get_order() >= start_order))
		{
			if (scrollx>img_width) scrollx=0;
		
			int w0 = img_width - scrollx;
			if (w0>240/2) w0=240/2;
			int w1 = 240/2-w0;
			if (w1<0) w1=0;
			{
				const u32* src_linep = (const u32*)wall_raw;
				u32* dst_linep = (u32*)fb::bb;
				for (int y=0; y<160; y++, src_linep+=img_width, dst_linep+=240/4) {
					CpuFastSet(src_linep+scrollx, dst_linep, COPY32|w0);			
					if (w1) CpuFastSet(src_linep, dst_linep+w0, COPY32|w1);			
				}
			}
			
			if (frameno>=0) {
				flikk.setFrame(frameno);
				flikk.drawMov(fb::bb, 1);
			}

			int lo = glow::intensity - (glow::range);
			int hi = glow::intensity + (glow::range);
			
			lo -= startfade<<10;
			hi -= startfade<<11;
			if (startfade>0) startfade--;
			
			if (pimp_get_order()==start_order+2) {
				lo -= (blinkdown<<10);
				if (lo<0) lo=0;
				hi = lo+(blinkdown<<11)+0x2000;
				blinkdown++;
				blinkdown+=(pimp_get_row()>>2);
			}
  
			if (lo > hi-0x3000) lo = hi-0x3000;
			int glow_hi = hi;// - (glow::range>>2);// - 0x2000;
			int glow_lo = (lo+glow_hi*3)>>2;//(lo+glow_hi)>>1;
		 	 


			if (swaps==0) VBlankIntrWait();
			fb::swap();
			exposure::update(lo, hi, glow_lo, glow_hi, hdr_wall_xpal+16*4, BG_COLORS+16, 256-16);
			if (swaps==0) {
				FORI(16) {
					int val = ((i - 1) * (0xFFFF / 15)) >> 8;
					if (i==0) val=0;
					if (val > 255) val = 255;
					BG_COLORS[i] = RGB8(val, val, val);
				}
			}
			glow::update(fb::fb, 0xB000);
			scrollx++;
			swaps++;
		}
	}

};
