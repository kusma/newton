// mapflip.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

void print_usage(void)
{
	printf(
		"usage: mapflip [options] file(s)\n"
		"options: -h horizontal flip\n"
		"         -v vertical flip\n"
		"         -w width, in tiles of map\n"
	);
}

#define DEFAULT_WIDTH 32 /* 256 pixels wide */

int main(int argc, char *argv[])
{
	std::vector<const char *> filenames;
	bool hflip = false;
	bool vflip = false;
	int width = DEFAULT_WIDTH;

	for (int i = 1; i < argc; ++i)
	{
		/* if the first charater is '-', it is a flag */
		if (argv[i][0] == '-')
		{
			/* if the second charater is not '-', it is a SHORT flag (one char) */
			if (argv[i][1] != '-')
			{
				/* check that the flag is not more than one character */
				if (argv[i][2] != '\0')
				{
					print_usage();
					exit(1);
				}

				if      (argv[i][1] == 'h') hflip = true;
				else if (argv[i][1] == 'v') vflip = true;
				else if (argv[i][1] == 'w')
				{
					if (i + 1 < argc)
					{
						width = atoi(argv[++i]);
					}
					else
					{
						print_usage();
						exit(1);
					}
				}
				else
				{
					print_usage();
					exit(1);
				}
			}
			else
			{
				/* if ever some multi-character versions of flags should be adde, this is the place */
				print_usage();
				exit(1);
			}
		}
		else
		{
			filenames.push_back(argv[i]);
		}
	}

	if (filenames.size() == 0)
	{
		fprintf(stderr, "*** ERROR: no files to flip\n");
		print_usage();
		exit(1);
	}

	printf(
		"hflip: %s\nvflip: %s\nwidth: %d\n",
		hflip == true ? "true" : "false",
		vflip == true ? "true" : "false",
		width
	);

	for (unsigned i = 0; i < filenames.size(); ++i)
	{
		printf("trying %s...\n", filenames[i]);
		FILE *fp = fopen(filenames[i], "rb");
		if (!fp)
		{
			fprintf(stderr, "*** ERROR: failed to open %s\n", filenames[i]);
			print_usage();
			exit(1);
		}

		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		rewind(fp);

		if (((size & 1) != 0) || ((size / 2) % width) != 0)
		{
			fprintf(stderr, "*** ERROR: file size not dividable by width (%d)\n", width);
			exit(1);
		}

		size /= 2;

		printf("size: %d\n", size);
		unsigned short *data = (unsigned short*)malloc(size * 2);
		if (!data)
		{
			fprintf(stderr, "*** ERROR: out of memory\n");
			exit(1);
		}

		size_t read = fread(data, 1, size * 2, fp);
		if (read != size * 2) fprintf(stderr, "*** WARNING: read-size mismatch\n");

		fclose(fp);

		fp = fopen(filenames[i], "wb");

		// add flip-bits
		for (int j = 0; j < size; ++j)
		{
			if (hflip) data[j] ^= (1 << 10);
			if (vflip) data[j] ^= (1 << 11);
		}

		int height = size / width;
//		printf("height: %d\n", height);

		for (int j = 0; j < height; ++j)
		{
//			printf("line %d\n", j);
			for (int k = 0; k < width; ++k)
			{
				// TODO: left-padding (x tiles)
				int x = hflip ? width  - k - 1 : k;
				int y = vflip ? height - j - 1 : j;

				assert(x >= 0);
				assert(x < width);
				assert(y >= 0);
				assert(y < height);

				fwrite(&data[y * width + x], 2, 1, fp);
			}
		}

		fclose(fp);
	}
	
	return 0;
}

