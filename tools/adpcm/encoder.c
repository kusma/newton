/*
 * adpcm.c --
 *
 *      Intel/DVI ADPCM coder/decoder.
 */

/***********************************************************
Copyright 1992 by Stichting Mathematisch Centrum, Amsterdam, The
Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.

STICHTING MATHEMATISCH CENTRUM DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH CENTRUM BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

#include "adpcm.h"

/* Intel ADPCM step variation table */
static int indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static int stepsizeTable[89] = {
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

void
adpcm_coder(
    const short *inp,
    unsigned char *outp,
    int len,
    struct adpcm_state *state)
{
    int val;			/* Current input sample value */
    int sign;			/* Current adpcm sign bit */
    int delta;			/* Current adpcm output value */
    int step;			/* Stepsize */
    int valprev;		/* virtual previous output value */
    int vpdiff;			/* Current change to valprev */
    int index;			/* Current step change index */
    int outputbuffer = 0;	/* place to keep previous 4-bit value */
    int bufferstep;		/* toggle between outputbuffer/output */

    valprev = state->valprev;
    index = state->index;
    step = stepsizeTable[index];

	bufferstep = 1;
	
	while (--len >= 0) {
#if 0
		val = *inp++;
#else
		val = *inp++ >> 8;
#endif
		/* Step 1 - compute difference with previous value */
		delta = val - valprev;
		sign = (delta < 0) ? 8 : 0;
		if ( sign ) delta = (-delta);
	
		delta = (delta<<2) / step;
		if ( delta > 7 ) delta = 7;
		vpdiff = ((delta*step) >> 2) + (step >> 3);
	
		/* Step 3 - Update previous value */
		if ( sign ) valprev -= vpdiff;
		else        valprev += vpdiff;
	
		/* Step 4 - Clamp previous value to 16 bits */
#if 0
		if      (valprev >  32767) valprev =  32767;
		else if (valprev < -32768) valprev = -32768;
#else
		if      (valprev >  127) valprev =  127;
		else if (valprev < -128) valprev = -128;
#endif

		/* Step 5 - Assemble value, update index and step values */
		delta |= sign;
	
		index += indexTable[delta];
		if ( index < 0 ) index = 0;
		if ( index > 88 ) index = 88;
		step = stepsizeTable[index];
	
		/* Step 6 - Output value */
		if ( bufferstep ) {
			outputbuffer = (delta << 4) & 0xf0;
		} else {
			*outp++ = (delta & 0x0f) | outputbuffer;
		}
		bufferstep = !bufferstep;
    }

    /* Output last step, if needed */
    if ( !bufferstep )
      *outp++ = outputbuffer;

    state->valprev = valprev;
    state->index = index;
}

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	unsigned short *indata = NULL;
	size_t indata_size = 0;
	
	unsigned char *outdata = NULL;
	size_t outdata_size = 0;
	
	struct adpcm_state state;
    state.valprev = 0;
    state.index = 0;
	
	if (argc != 3)
	{
		printf("USAGE: %s infile.raw outfile.adpcm\n", argv[0]);
		exit(1);
	}
	
	/* load infile */
	{
		FILE *fp = fopen(argv[1], "rb");
		if (!fp)
		{
			printf("failed to open %s for loading\n", argv[1]);
			exit(1);
		}
		
		fseek(fp, 0, SEEK_END);
		indata_size = ftell(fp);
		indata = malloc(indata_size);
		rewind(fp);
		fread(indata, 2, indata_size / 2, fp);
		fclose(fp);
	}
	
	/* allocate outdata */
	outdata_size = indata_size / 4;
	outdata = malloc(outdata_size);
	
	adpcm_coder(indata, outdata, indata_size / 2, &state);
	
	/* free indata */
	free(indata);
	indata = NULL;
//	indata_size = 0;
	
	/* store outfile */
	{
		FILE *fp = fopen(argv[2], "wb");
		if (!fp)
		{
			printf("failed to open %s for writing\n", argv[1]);
			exit(1);
		}
		
		fwrite(outdata, 1, outdata_size, fp);
		fclose(fp);
	}

	{
		FILE *fp;
		char *decoded = malloc(((indata_size + 1) & ~1) / 2);
		if (NULL != decoded)
		{
			adpcm_init(outdata);
			adpcm_render(decoded, indata_size / 2);
			fp = fopen("output.raw", "wb");
			if (NULL != fp)
			{
				fwrite(decoded, indata_size / 2, 1, fp);
				fclose(fp);
			}
		}
	}

	/* free outdata */
	free(outdata);
	outdata = NULL;
	outdata_size = 0;

	return 0;
}
