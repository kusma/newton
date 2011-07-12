#include "part.h"
#include "fb.h"
#include <pimp_gba.h>

static part::callback init_func = NULL;
static part::callback vblank_func = NULL;
static part::callback next_vblank_func = NULL;

void part::vblank(void)
{
	pimp_gba_vblank();
	if (NULL != vblank_func) vblank_func();
	pimp_gba_frame();
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

