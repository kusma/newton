#ifndef LINEARCURVE_H
#define LINEARCURVE_H

#include "fixed.h"
#include <stdlib.h>

class LinearCurve
{
public:
	struct keyframe
	{
		fixed16 time;
		fixed16 value;
	};
	
	LinearCurve(const struct keyframe *data, const size_t data_size)
		: data(data), data_size(data_size), last_pos(0)
	{
		assert(0 < data_size);
	}
	
	fixed16 getValue(fixed16 time)
	{
		assert(0 != data_size);
		assert(0 <= last_pos);
		assert(data_size > last_pos);
		
		/* scan forward */
		while ((last_pos < (data_size - 1)) && (data[last_pos + 1].time < time)) last_pos++;
		
		/* scan backward */
		while ((last_pos > 0) && (data[last_pos].time > time)) last_pos--;

		assert(0 <= last_pos);
		assert(data_size > last_pos);
		
		fixed16 v = data[last_pos].value;
		
		if (last_pos >= (data_size - 1)) return v; /* clamp at "right" end */
		if ((last_pos == 0) && (data[last_pos].time > time)) return v; /* clamp at "left"  end */
		
		fixed16 t = (time - data[last_pos].time) / (data[last_pos + 1].time - data[last_pos].time);
		fixed16 d = data[last_pos + 1].value - data[last_pos].value;
		
		v += d * t;		
		return v;
	}
	
private:
	int last_pos;
	
	const struct keyframe *data;
	const size_t data_size;
};

#define LINEARCURVE(array) LinearCurve(array, sizeof(array) / sizeof(array[0]))

#endif /* LINEARCURVE_H */
