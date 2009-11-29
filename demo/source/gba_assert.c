#include <assert.h>
#include <stdio.h>

#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <mappy.h>

void __assert(const char *file, const int line, const char *msg)
{
#if 1
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleInit(0, 4, 0, NULL, 0, 15);

	BG_COLORS[0] = RGB5(0, 0, 0);
	BG_COLORS[241] = RGB5(31, 31, 31);
	REG_DISPCNT = MODE_0 | BG0_ON;
	
	printf("assertation failed \"%s\" (%s:%d)\n", msg, file, line);
	
	while (1);
	
#else

	/* give vba a nice log-message */
	dprintf("assertation failed \"%s\" (%s:%d)\n", msg, file, line);

	/* make sure we're in the right mode */
	SetMode(MODE_4 | BG2_ENABLE);

	/* clear the front buffer to color 0, in case it has been modified */
	u32 col = 0;
	CpuFastSet(&col, MODE5_FB, DMA_SRC_FIXED | ((240 * 160) / 4));

	while (1)
	{
		/* wait for vsync, clear screen to pink, wait for the end of vsync */
		while (REG_VCOUNT != 160);
		BG_COLORS[0] = RGB5(31, 0, 31);
		while (REG_VCOUNT != 0);

		/* wait for vsync, clear screen to green, wait for the end of vsync */
		while (REG_VCOUNT != 160);
		BG_COLORS[0] = RGB5(0, 31, 0);
		while (REG_VCOUNT != 0);
	}
#endif
}
