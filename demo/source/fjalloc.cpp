#include "fjalloc.h"
#include "support.h"
#include <gba_systemcalls.h>
#include "fb.h"

#define FJALLOC_SIZE (32*1024)
u32 bigtable[FJALLOC_SIZE] EWRAM_DATA;
u32* p = bigtable;

namespace fjalloc {
	static void feil() {
		while(1) {
			static int tall;
			tall++;
			CpuFastSet(&tall, fb::fb, FILL | (240*160/4));
		}
	}
	void* get(int size) {
		int size32 = (size + 3)>>2;
		//dprintf("allocing %i ", size32);
		u32* oldp = p;
		p += size32;
		if (p >= bigtable+FJALLOC_SIZE) feil();
		//dprintf("ok!\n");
		return oldp;
	}
	void release(int size) {
		int size32 = (size + 3)>>2;
		//dprintf("deallocing %i\n", size32);
		p -= size32;
		if (p < bigtable) feil();		
	} 
} 
