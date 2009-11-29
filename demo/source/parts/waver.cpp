#include "exstructs.h"

#include <gba_interrupt.h>
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


extern u16 wave_pal[];

extern u8 wave_1[];
extern u8 wave_2[];
extern u8 wave_3[];
extern u8 wave_4[];


static int boost=40;

static int blanks = 0;
static int bg_fade_speed = 800;
static int bg_fade = 0;
static int fuzz = 256; 
static void vblank_fra_newton() {
	boost += ((int(cheap_rand()>>16) & 0xFF) - 128)>>4;
	if (boost>86) boost=86;
	if (boost<-0) boost=-0;
	
	boost = (fixed_cos(blanks*0xB0)+0x14000)>>12;

	palette::color_boost(fb::temppal, wave_pal, 256, boost);


	BG_COLORS[0]=0;

	int shade = (fixed_sin(blanks*0xD7)>>3) + 0x10000;

	int tmp = MUL16(bg_fade,shade);
	if (tmp>0x10000) tmp=0x10000;
	if (tmp<0) tmp=0;
	palette::boost(BG_COLORS+1, fb::temppal+1, 255, (tmp - 0x10000)>>8 );

	bg_fade += bg_fade_speed;
	if (bg_fade>0x10000) bg_fade=0x10000;
	if (bg_fade<0) bg_fade=0;

}

void global_vblank();
static void vblank()
{
	pimp_vblank();
	pimp_frame();
	global_vblank();
	vblank_fra_newton();
	blanks++;
}

static void callback(int type, int data)
{
}


namespace parts {
	void wave(int len)
	{
		pimp_set_callback(callback);
		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);

		DisableInterrupt(IE_HBL);		
		REG_DMA0CNT = 0;

		int start=pimp_get_order();
		int framei=0;
		int swaps=0;
		int img_width=356/2;
		while (pimp_get_order()<start+len) {
									
			
			VBlankIntrWait();
			
			if (pimp_get_row() > 0x10) bg_fade_speed = -600;
			
	
			if ((swaps&7) == 0) {
				framei++;
				u8* buff = NULL;
				if (framei>3) framei=0;
				if (framei==0) buff=wave_1;
				if (framei==1) buff=wave_2;
				if (framei==2) buff=wave_3;
				if (framei==3) buff=wave_2;
				CpuFastSet(buff, fb::bb, COPY32 | (GBA_WIDTH*GBA_HEIGHT/4));
				fb::swap();
			}

			swaps++;
		}
	}
};
