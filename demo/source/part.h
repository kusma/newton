#ifndef PART_H
#define PART_H

namespace part
{
	typedef void (*callback)(void);
	void vblank(void);
	void set_part(callback init_func, callback vblank_func);
	void swap();
}

#endif /* PART_H */
