#pragma once
#include <gba_video.h>
#include <gba_dma.h>	
#include <mappy.h>
//#include <stdlib.h>

// 64-bit typer
typedef unsigned long long u64;
typedef signed long long   s64;
typedef volatile bool	   vbool;

// noen registre som mangla
#define REG_WAITCNT *(vs16*)(REG_BASE + 0x0204)

#define FORI(a)   for (int i=0; i<a; i++) 
#define FORJ(a)   for (int j=0; j<a; j++) 
#define FORXY(a, b) for (int y=0; y<b; y++) for (int x=0; x<a; x++)  

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define MUL16(a, b)         s32((s64(a)*s64(b))>>16)

