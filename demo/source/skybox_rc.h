#pragma once
#include "support.h"
#include "vector3.h"


namespace skybox_rc {

	// Draw raycast skybox.
	// (Normal projection; from cubemap to planar projection)
	void draw(u8* dst, fixed16 dstWidth_rcp, fixed16 dstHeight_rcp, u8** cubemap, bool hq, int texDim_l2, Vector3 v, Vector3 dvdx, Vector3 dvdy);
	
	
	// Draw raycast skybox. 
	// This one projects form cubemap to horizontal cylinder.
	// It can be used to render a vertical angle into a cylindermap 
	// for use with the sprite-based skycylinder engine
	// void draw_cylinder(...)
};
