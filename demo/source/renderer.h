#ifndef RENDERER_H
#define RENDERER_H

#include "vertexcache.h"
#include "vertextransformer.h"

class Renderer
{
public:
	Renderer() : vertex_cache(VertexTransformer())
	{
		
	}
	
// private:
	Matrix4x4 mvp;
	VertexCache<Vertex, VertexTransformer> vertex_cache;
};

#endif /* RENDERER_H */
