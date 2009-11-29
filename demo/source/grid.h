#ifndef _GRID_H_
#define _GRID_H_

#include "base.h"

typedef struct {
	s32 u, v;
	int col;
} grid_node;

// #undef HEIGHT
// #define HEIGHT (64 + 16)

#define GRID_NODE_LOG2_WIDTH 4 // 8
#define GRID_NODE_LOG2_HEIGHT 4 // 8
#define GRID_NODE_WIDTH (1 << GRID_NODE_LOG2_WIDTH)
#define GRID_NODE_HEIGHT (1 << GRID_NODE_LOG2_HEIGHT)
#define GRID_WIDTH ((GBA_WIDTH / GRID_NODE_WIDTH) + 1)
#define GRID_HEIGHT ((GBA_HEIGHT / GRID_NODE_HEIGHT) + 1)

typedef grid_node grid[GRID_WIDTH * GRID_HEIGHT];

void draw_grid(u8 *target, grid g, const u8 *const texture) IWRAM_CODE;
void draw_parallax_grid(u8 *target, grid g, const u8 *const texture, const u8 *height) IWRAM_CODE;

#endif /* _GRID_H_ */
