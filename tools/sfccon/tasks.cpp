#include "tasks.h"
#include <stdlib.h>

void task_shadetable();
void task_luts();
void task_glow_lut();
void task_pic2tex();
void task_pic2nmap();
void task_pic2ebump();
void task_actpal();
void task_gbapal();
void task_gbaxpal();
//void task_fadepal();
//void task_pixglowpal();
void task_shiftpal();
void task_shiftprepal();
void task_pic2mask();
void task_pic2symraw();
void task_flikk();
void task_dblflikk();
void task_raster();
void task_reduce();
void task_quantisize();
void task_tablefx();
void task_amigapal();
//void task_quantisize_xpal();
void task_glow_precalc();


Task tasks[TASKCOUNT] = {
	{ task_luts,		"luts",			INPUT_NONE,		0 },
	{ task_glow_lut,	"glow_lut",		INPUT_NONE,		1 },
	{ task_pic2tex,		"pic2tex",		INPUT_IMAGE,	1 },
	{ task_pic2nmap,	"pic2nmap",		INPUT_IMAGE,	1 },
	{ task_pic2ebump,	"pic2ebump",	INPUT_IMAGE,	1 },
	{ task_actpal,		"actpal",		INPUT_ACT,		1 },
	{ task_gbapal,		"gbapal",		INPUT_ACT,		1 },
	{ task_gbaxpal,		"gbaxpal",		INPUT_ACT,		1 },
	{ task_shiftpal,	"shiftpal",		INPUT_ACT,		4 },
	{ task_shiftprepal,	"shiftprepal",	INPUT_NONE,		2 },
	{ task_pic2mask,	"pic2mask",		INPUT_IMAGE,	1 },
	{ task_raster,      "raster",       INPUT_NONE,     0 },
	{ task_reduce,      "reduce",       INPUT_NONE,    -3 },
	{ task_quantisize,  "quantisize",   INPUT_ACT,      3 },
	{ task_tablefx,     "tablefx",      INPUT_NONE,     0 },
	{ task_amigapal,    "amigapal",     INPUT_ACT,      1 },
	{ task_glow_precalc, "glow_precalc",INPUT_ANIM,     5 },
	{ task_flikk,       "flikk",        INPUT_ANIM,     2 },
	{ task_shadetable,  "shadetable",   INPUT_ACT,      3 }
};

const char* taskDescs[TASKCOUNT] = {
	/* luts			*/ "generate c-code with some luts",
	/* pic2raw		*/ "generate some datafiles for glow effect\n   arg1: target directory",
	/* pic2raw		*/ "create TEXTURE from picture ",
	/* pic2nmap		*/ "create NMAP from picture",
	/* pic2ebump	*/ "create EBUMP from picture",
	/* actpL		*/ "save act palette",
	/* gbapal		*/ "save gba palette",
	/* gbaxpal		*/ "save gba HDR xpal",
	/* shiftpal		*/ "make shiftable palette.\n   arg2: color  RRGGBB\n   arg3: bright    0.0 -> 1.0\n   arg4: reserved count (reserve colors at end of pal)",
	/* shiftprepal	*/ "make 8-color preview palette.\n   arg1: color  RRGGBB\n   arg2: bright    0.0 -> 1.0",
	/* pic2mask		*/ "create RLE-encoded mask from 8-color picture",
	/* raster       */ "make some lut files for rasterizer",
	/* reduce       */ "make a floating point palette\n  arg1: Number output colors\n  arg2: number of reserved colors\n  more args: list of images",
	/* quantisize   */ "index image using normal palette\n  arg1: palette\n  arg2: image\n  arg3: 0=normal, 1=noise dither",
	/* tablefx      */ "mekk tunnelltabell",
	/* amigapal     */ "save amiga palette",
	/* glow_precalc */ "anim to glow precalc\n   arg2: threshold\n   arg3: gain\n   arg4: blur\n   arg5: frameskip (1)",
	/* flikk        */ "anim to flikk",
	/* shadetable   */ "weight shadetable\n   arg2: weight (of color index in MSB)\n   arg3: debug (0)",
};
