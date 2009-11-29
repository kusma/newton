#ifndef VERTEXTRANSFORMER_H
#define VERTEXTRANSFORMER_H

#include "vector3.h"
#include "vector4.h"
#include "matrix4x4.h"

class Vertex
{
public:
	Vector4 pos;
	int sx, sy;
};

class VertexTransformer
{
public:
	
	VertexTransformer() : vertex_array(NULL)
	{
		/* nuthin' */
	}
	
	/* given an input-vertex index, transform the vertex belonging to it */
	Vertex operator ()(int index)
	{
		ASSERT(NULL != vertex_array);
		
		Vertex v;
		Vector4 pos = vertex_array[index];
		fixed16 w_rcp = rcp(pos.w);
		v.sx = (SCREEN_WIDTH  * 256 / 2) + int(pos.x * w_rcp * (SCREEN_WIDTH  * 256 / 2));
		v.sy = (SCREEN_HEIGHT * 256 / 2) + int(pos.y * w_rcp * (SCREEN_HEIGHT * 256 / 2));
		return v;
	}
// private:
	const Vector4 *vertex_array;
};

#endif /* VERTEXTRANSFORMER_H */
