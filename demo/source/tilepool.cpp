#include "tilepool.h"

void TilePool::reset()
{
	first_free  = &tiles[0];
	first_taken = NULL;
	
	for (int i = 0; i < MAX_TILES; ++i)
	{
		tiles[i].index = i;
		tiles[i].prev  = i > 0 ? &tiles[i - 1] : NULL;
		tiles[i].next  = i < (MAX_TILES - 1) ? &tiles[i + 1] : NULL;
	}
}

int TilePool::getTile()
{
	/* out of tiles */
	if (NULL == first_free) return -1;

	/* just some sanity-checks */
	if (NULL != first_free)  ASSERT(first_free->prev == NULL);
	if (NULL != first_taken) ASSERT(first_taken->prev == NULL);

	/* get tile */
	struct tile *t = first_free;
	ASSERT(t->prev == NULL);
	
	/* remove from free list */
	if (first_free->next != NULL) first_free->next->prev = NULL; 
	first_free = t->next;
	
	
	/* move to taken-list */
	if (first_taken != NULL) first_taken->prev = t;
	t->next = first_taken;
	t->prev = NULL;
	first_taken = t;
	
	/* return index */
	return t->index;
}

void TilePool::freeTile(int tile)
{
	if (tile < 0) return;

	/* get tile */
	struct tile *t = &tiles[tile];
	
	/* connect next and prev (remove from taken list) */
	if (NULL != t->prev) t->prev->next = t->next;
	else
	{
		if (NULL != t->next) t->next->prev = NULL;
		first_taken = t->next;
	}
	if (NULL != t->next) t->next->prev = t->prev;
	
	/* insert in front of free list */
	if (NULL != first_free) first_free->prev = t;
	t->prev = NULL;
	t->next = first_free;
	first_free = t;
}
