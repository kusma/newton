#pragma once
#include "support.h"

namespace vcount
{
	typedef void (*VCountFn)(int y, int param);

	void clear();
	void insert(int y, VCountFn fn, int param=0);
	void vblank();
};
