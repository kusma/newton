#include "adpcm.h"

#define IWRAM_DATA
#define NULL ((void*)0)

const signed char indexTable[16] IWRAM_DATA = 
{
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

const unsigned short stepsizeTable[89] IWRAM_DATA = 
{
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static const unsigned char* adpcm_data = NULL;
static int adpcm_valprev;
static int adpcm_index;

void adpcm_init(void *adpcm_data_)
{
	adpcm_data = (unsigned char*)adpcm_data_;
	adpcm_index = 0;
}

void adpcm_render(signed char *dst, int len)
{
    int sign;
    int delta;
    int step;
    int vpdiff;
	
	/* fetch states */
	const unsigned char *data = adpcm_data;
	int valprev = adpcm_valprev;
	int index   = adpcm_index;
	
	if (NULL == data) return;
	
    step = stepsizeTable[index];

	len >>= 1;

	
    for ( ; len > 0 ; len-- )
	{
		int sample;
		
		/* fetch data */
		int inputbuffer = *data++;
		
		/* first sample */
		/* extract top 4 bits */
		delta = (inputbuffer >> 4) & 0xF;
		
		/* process */
		index += indexTable[delta];
		if (index < 0)  index = 0;
		if (index > 88) index = 88;
		sign = delta & 8; /* top bit */

		vpdiff = (((delta & 7) * step) >> 2) + (step >> 3);
		if (sign) vpdiff = -vpdiff;
		valprev += vpdiff;
		
		/* clamp and store */
		if      (valprev >  127) valprev =  127;
		else if (valprev < -128) valprev = -128;
		*dst++ = valprev;
		
		step = stepsizeTable[index];

		/* second sample */
		/* extract bottom 4 bits */
		delta = inputbuffer & ((1 << 4) - 1);
		
		/* process */
		index += indexTable[delta];
		if (index < 0)  index = 0;
		if (index > 88) index = 88;
		sign = delta & 8; /* top bit */

		vpdiff = (((delta & 7) * step) >> 2) + (step >> 3);
		if (sign) vpdiff = -vpdiff;
		valprev += vpdiff;
		
		/* clamp and store */
		if      (valprev >  127) valprev =  127;
		else if (valprev < -128) valprev = -128;
		*dst++ = valprev;

		step = stepsizeTable[index];
	}
	
	/* store states back */
	adpcm_data = data;
	adpcm_valprev = valprev;
	adpcm_index = index;
}
