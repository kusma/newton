#pragma once
#include "support.h"

namespace flikkfx {
	void cheap_blur(u8* dst_buff, u8* src_buff);
	void overlay(u8* dst, u8* src, u32 scrolly, u8* img, int img_height);
};
