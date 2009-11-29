#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_timers.h>

#ifndef REG_WAITCNT
#define REG_WAITCNT (*(vu16*)(REG_BASE + 0x0204))
#endif

#include <stdio.h>

#include "adpcm.h"

extern char test_adpcm[];

int accum = 0;
void mix()
{
	REG_TM2CNT_H = 0;
	REG_TM2CNT_L = 0;
	REG_TM2CNT_H = TIMER_START | 2;
		
	BG_COLORS[0] = RGB5(31, 0, 0);
	adpcm_gba_frame();
	BG_COLORS[0] = RGB5(0, 0, 0);
	
	u32 value = REG_TM2CNT_L;
//	iprintf("cpu: (%d c/f)\n", value);
	accum += value << 8;
}

void report()
{
	float percent = (float)(accum * 100) / (1 << 24);
	iprintf("cpu: %d.%03d%% (%d c/f)\n", (int)percent, (int)(percent * 1000) % 1000, accum / 60);
	accum = 0;
}

void timer3()
{
	report();
}

void vblank()
{
	adpcm_gba_vblank();
	mix();
}

int main()
{
	REG_WAITCNT = 0x46d6; // lets set some cool waitstates...
//	REG_WAITCNT = 0x46da; // lets set some cool waitstates...

	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleInit(0, 4, 0, NULL, 0, 15);

	BG_COLORS[0] = RGB5(0, 0, 0);
	BG_COLORS[241] = RGB5(31, 31, 31);
	REG_DISPCNT = MODE_0 | BG0_ON;
	
	adpcm_gba_init(test_adpcm);

	irqSet(IRQ_TIMER3, timer3);
	irqEnable(IRQ_TIMER3);
	REG_TM3CNT_L = 0;
	REG_TM3CNT_H = TIMER_START | TIMER_IRQ | 2;

	irqSet(IRQ_VBLANK, vblank);
	irqEnable(IRQ_VBLANK);
	
	while (1)
	{
		VBlankIntrWait();
	}
	
	return 0;
}
