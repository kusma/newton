#include <stdio.h>
#include "adpcm.h"

#define SIZE 56

unsigned char outdata[SIZE];
unsigned char indata[SIZE / 2];

int main()
{
	int i;
	
	for (i = 0; i < SIZE / 2; ++i)
	{
		indata[i] = (rand() * rand()) & 0xFF;
	}
	
	adpcm_init(indata);
	adpcm_render(outdata, SIZE);
	
	for (i = 0; i < SIZE; ++i)
	{
		printf("%02X ", outdata[i]);
	}
	printf("\n");
	
	return 0;
}
