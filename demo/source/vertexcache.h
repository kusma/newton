#ifndef VERTEXCACHE_H
#define VERTEXCACHE_H

#define VERTEX_CACHE_LOG2_SIZE 8

/* derived constants - do not touch */
#define VERTEX_CACHE_SIZE (1 << VERTEX_CACHE_LOG2_SIZE)
#define VERTEX_CACHE_MASK (VERTEX_CACHE_SIZE - 1)

template <typename T, class transformer>
class VertexCache
{
public:
	VertexCache(transformer trans)
	: trans(trans)
	{
		invalidate();
	}
	
	void invalidate()
	{
		for (int i = 0; i < VERTEX_CACHE_SIZE; ++i)
		{
			elems[i].index = -1;
		}
	}
	
	bool hit(int index)
	{
		int key = index & VERTEX_CACHE_MASK;
		return elems[key].index == index;
	}
	
	const T getVertex(int index)
	{
		int key = index & VERTEX_CACHE_MASK;
		if (elems[key].index == index) return elems[key].data;
		
		/* not found, transform vertex instead */
		elems[key].data  = trans(index);
		elems[key].index = index;
		return elems[key].data;
	}
	
private:
	struct {
		int index;
		T data;
	} elems[VERTEX_CACHE_SIZE];

public:	
	transformer trans;
};

#endif /* VERTEXCACHE_H */
