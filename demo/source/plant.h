#ifndef PLANT_H
#define PLANT_H

#include "matrix4x4.h"
#include "vector3.h"
#include "vector4.h"

const int PLANT_MAX_BRANCHES = 64;
class Branch
{
public:
	Vector3 origin;
	Vector3 target;
	Vector3 curr;
	int frame;
};

class Plant
{
public:
	Plant()
	{
		reset();
	}

	void grow(fixed16 amt)
	{
		if (visible_branch_count < (branch_count - 1))
		{
			Branch &b = branches[visible_branch_count - 1];
			b.curr = b.origin + ((b.target - b.origin) * amt);
		}
//		b.curr = b.target + ((b.origin - b.target) * amt); 
	}

	void advance()
	{
		if (visible_branch_count < (branch_count - 1)) visible_branch_count++;
	}

	void reset()
	{
		visible_branch_count = 1;

		/* default curr to origin */
		for (int i = 0; i < branch_count; ++i)
		{
			branches[i].curr = branches[i].origin;
		}
	}

	void draw(u8 *bb, const Matrix4x4 &mvp, const u8 *texture);


	Branch branches[PLANT_MAX_BRANCHES];
	int visible_branch_count;
	int branch_count;
};

#endif /* PLANT_H */
