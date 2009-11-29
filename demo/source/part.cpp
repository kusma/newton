#include "part.h"
#include "fb.h"
#include "adpcm.h"

static part::callback init_func = NULL;
static part::callback vblank_func = NULL;
static part::callback next_vblank_func = NULL;

void part::vblank(void)
{
	adpcm_gba_vblank();
	if (NULL != vblank_func) vblank_func();
	adpcm_gba_frame();
}

void part::swap()
{
	if (NULL != init_func)
	{
		init_func();
		init_func = NULL;
		vblank_func = next_vblank_func;
		next_vblank_func = NULL;
	}
	
	fb::swap();
}

void part::set_part(part::callback init_func_, part::callback vblank_func_)
{
	init_func = init_func_;
	vblank_func = NULL;
	next_vblank_func = vblank_func_;
}

