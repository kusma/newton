#include "tasks.h"
#include <stdlib.h>

void task_luts();
void task_glow();
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
//void task_quantisize_xpal();

Task tasks[TASKCOUNT] = {
	{ task_luts,		"luts",			INPUT_NONE,		0 },
	{ task_glow,		"glow",			INPUT_NONE,		1 },
	{ task_pic2tex,		"pic2tex",		INPUT_IMAGE,	1 },
	{ task_pic2nmap,	"pic2nmap",		INPUT_IMAGE,	1 },
	{ task_pic2ebump,	"pic2ebump",	INPUT_IMAGE,	1 },
	{ task_actpal,		"actpal",		INPUT_ACT,		1 },
	{ task_gbapal,		"gbapal",		INPUT_ACT,		1 },
	{ task_gbaxpal,		"gbaxpal",		INPUT_ACT,		1 },
//	{ task_fadepal,     "fadepal",      INPUT_ACT,      1 },
//	{ task_pixglowpal,  "pixglowpal",   INPUT_ACT,      1 },
	{ task_shiftpal,	"shiftpal",		INPUT_ACT,		4 },
	{ task_shiftprepal,	"shiftprepal",	INPUT_NONE,		2 },
	{ task_pic2mask,	"pic2mask",		INPUT_IMAGE,	1 },
	{ task_pic2symraw,	"pic2symraw",	INPUT_IMAGE,	1 },
	{ task_flikk,		"flikk",		INPUT_FLC,		3 },
	{ task_dblflikk,	"trplflikk",	INPUT_FLC,		5 },
	{ task_raster,      "raster",       INPUT_NONE,     0 },
	{ task_reduce,      "reduce",       INPUT_NONE,    -2 },
//	{ task_quantisize_xpal,  "quantisize_xpal",   INPUT_NONE,     3 },
	{ task_quantisize,  "quantisize",   INPUT_ACT,      3 }
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
//	/* fadepal		*/ "save gba palette for fademask",
//	/* pixglowpal	*/ "save gba palette for pixel glow",
	/* shiftpal		*/ "make shiftable palette.\n   arg2: color  RRGGBB\n   arg3: bright    0.0 -> 1.0\n   arg4: reserved count (reserve colors at end of pal)",
	/* shiftprepal	*/ "make 8-color preview palette.\n   arg1: color  RRGGBB\n   arg2: bright    0.0 -> 1.0",
	/* pic2mask		*/ "create RLE-encoded mask from 8-color picture",
	/* pic2symraw   */ "something for symbian os (bleh)",
	/* flikk        */ "convert flc/fli to flikk\n	arg2: 0/1, shadow?\n   arg3: 0.0-1.0 huff thresh (good with 0.8)",
	/* dblflikk		*/ "convert flc/fli and mask to shadowflikk\n	arg2: multiplicativ fli/flx.\n	arg3: 0/1, shadow?   arg4: 0.0-1.0 huff thresh (good with 0.8)",
	/* raster       */ "make some lut files for rasterizer",
	/* reduce       */ "make a floating point palette\n  arg1: Number output colors\n  more args: list of images",
//	/* quantisize_xpal */ "index image using HDR palette\n  arg1: palette\n  arg2: image\n  arg3: 0=normal, 1=noise dither",
	/* quantisize   */ "index image using normal palette\n  arg1: palette\n  arg2: image\n  arg3: 0=normal, 1=noise dither",
};
