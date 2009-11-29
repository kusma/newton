//
// TinyPTC by Gaffer
// www.gaffer.org/tinyptc
//
extern "C"
{
#include "tinyptc.h"
}

#include "base.h"

#define _USE_MATH_DEFINES
#include <windows.h>
#include <math.h>
#include <stdio.h>

float randf()
{
	return float(rand()) / RAND_MAX;
}

void flip(u32 *fb, u8 *bb, u16 *pal)
{
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			int idx = bb[x + y * SCREEN_WIDTH];
			ASSERT(idx >= 0);
			ASSERT(idx < 256);

			u32 c = u32(pal[idx]);

			u32 r = (c >>  0) & ((1 << 5) - 1);
			u32 g = (c >>  5) & ((1 << 5) - 1);
			u32 b = (c >> 10) & ((1 << 5) - 1);

			r <<= 3;
			g <<= 3;
			b <<= 3;

			c = b | (g << 8) | (r << 16);

			fb[x + y * SCREEN_WIDTH] = c;
		}
	}
}

static u32 fb[SCREEN_WIDTH * SCREEN_HEIGHT];
static u8  bb[SCREEN_WIDTH * SCREEN_HEIGHT];

#include <freeimage.h>

double frac(double f)
{
	return f - floor(f);
}

#include "image.h"
#include <algorithm>

RGBQUAD sample(Image &bmp, int x, int y)
{
	RGBQUAD res;

	/* clamp coordinates */
	if (x < 0) x = 0;
	if (x >= bmp.GetWidth() - 1)  x = bmp.GetWidth() - 1;
	if (y < 0) y = 0;
	if (y >= bmp.GetHeight() - 1) y = bmp.GetHeight() - 1;

	FreeImage_GetPixelColor(bmp.GetFIBitmap(), x, y, &res);
	return res;
}

RGBQUAD bilinear_sample(Image &bmp, double x, double y)
{
	RGBQUAD res;
	RGBQUAD v[4];
	RGBQUAD h[2];

	x *= bmp.GetWidth() - 1;
	y *= bmp.GetHeight() - 1;

	v[0] = sample(bmp, int(x) + 0, int(y) + 0);
	v[1] = sample(bmp, int(x) + 1, int(y) + 0);
	v[2] = sample(bmp, int(x) + 0, int(y) + 1);
	v[3] = sample(bmp, int(x) + 1, int(y) + 1);

	/* interpolate red component */
	h[0].rgbRed = BYTE(v[0].rgbRed + (v[1].rgbRed - v[0].rgbRed) * frac(x));
	h[1].rgbRed = BYTE(v[2].rgbRed + (v[3].rgbRed - v[2].rgbRed) * frac(x));
	res.rgbRed  = BYTE(h[0].rgbRed + (h[1].rgbRed - h[0].rgbRed) * frac(y));

	/* interpolate green component */
	h[0].rgbGreen = BYTE(v[0].rgbGreen + (v[1].rgbGreen - v[0].rgbGreen) * frac(x));
	h[1].rgbGreen = BYTE(v[2].rgbGreen + (v[3].rgbGreen - v[2].rgbGreen) * frac(x));
	res.rgbGreen  = BYTE(h[0].rgbGreen + (h[1].rgbGreen - h[0].rgbGreen) * frac(y));

	/* interpolate blue component */
	h[0].rgbBlue = BYTE(v[0].rgbBlue + (v[1].rgbBlue - v[0].rgbBlue) * frac(x));
	h[1].rgbBlue = BYTE(v[2].rgbBlue + (v[3].rgbBlue - v[2].rgbBlue) * frac(x));
	res.rgbBlue  = BYTE(h[0].rgbBlue + (h[1].rgbBlue - h[0].rgbBlue) * frac(y));
	
	return res;
}

#define DIST_EXP 2.0
#define SUPERSAMPLE 2
#define TEXTURE_HEIGHT 1024

int main()
{
	/*	printf("%f\n", floor(0.9 + 0.5)); */
	if (!ptc_open("test", SCREEN_WIDTH, SCREEN_HEIGHT)) return 1;

	Image bmp;
	if (!bmp.Load("image.png"))
	{
		printf("error: failed to load image\n");
		exit(1);
	}

	FIBITMAP *tex = FreeImage_Allocate(256, TEXTURE_HEIGHT, 24);
	if (NULL == tex)
	{
		printf("error: failed to create image\n");
		exit(1);
	}
	
	const u8 dithermask[4][4] = 
	{
		{ 0x11, 0x99, 0x33, 0xbb },
		{ 0xdd, 0x55, 0xff, 0x77 },
		{ 0x44, 0xcc, 0x22, 0xaa },
		{ 0xff, 0x88, 0xee, 0x66 }
	};

	/* setup zoom-table */
	u8 zoom_table[(SCREEN_WIDTH / 2) * SCREEN_HEIGHT * 2];
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH / 2; ++x)
		{
			/* normalized integer coordinate */
			int ilx = x - SCREEN_WIDTH / 2;
			int ily = y - SCREEN_HEIGHT / 2;

			/* un-normalized floating point coordinate */
			double lx = double(ilx) / (SCREEN_WIDTH / 2);
			double ly = double(ily) / (SCREEN_HEIGHT / 2);

			/* make sure center is not inside first pixel, but rather between the four center ones */
			lx += (0.5 / (SCREEN_WIDTH / 2));
			ly += (0.5 / (SCREEN_HEIGHT / 2));

			/* calculate distance */
			double dist = sqrt((lx * lx) + (ly * ly));

			/* make top left corner distance 1.0 */
			dist /= sqrt(2.0);

			/* get dither-amounts */
			float dither_dist = 0.0f; //float(dithermask[y & 3][x & 3]) / 255;
			float dither_deg  = float(dithermask[x & 3][y & 3]) / 255 - 0.5f;

			/* make integer coordinate */
			int idist = int(floor(dist * 255.0 + 0.5 + dither_dist));
			idist &= 255;

			/* calculate rotation */
			float deg = atan2(float(lx), float(ly));

			/* integer coordinate*/
			int ideg  = int(floor(deg * (256.0 / (2 * M_PI)) + dither_deg));
			ideg  &= 0xFF;

			/* store in LUT */
			zoom_table[(x + y * (SCREEN_WIDTH / 2)) * 2 + 0] = ideg;
			zoom_table[(x + y * (SCREEN_WIDTH / 2)) * 2 + 1] = idist;
		}
	}

#if 0
	/* test xor-trick */
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH / 2; ++x)
		{

			int ideg1, idist1;
			int ideg2, idist2;

			{
				int ilx = x - SCREEN_WIDTH / 2 + 1;
				int ily = y - SCREEN_HEIGHT / 2 + 1;
				double lx = double(ilx) / (SCREEN_WIDTH / 2);
				double ly = double(ily) / (SCREEN_HEIGHT / 2);

				lx += (0.5 / SCREEN_WIDTH);
				ly += (0.5 / SCREEN_HEIGHT);

//				printf("%f %f\n", lx, ly);
				ly = -ly;
				double dist = sqrt((lx * lx) + (ly * ly));
				dist /= sqrt(2.0);

				idist1 = int(dist * 255);
				idist1 &= 255;

				float deg = atan2(float(lx), float(ly));
				ideg1  = int(floor(deg * (255 / (2 * M_PI))));
				ideg1  &= 0xFF;
				ideg1  ^= 0xFF;
//				ideg1  += 1;
			}

			{
				int ilx = x - SCREEN_WIDTH / 2 + 1;
				int ily = y - SCREEN_HEIGHT / 2 + 1;
				double lx = double(ilx) / (SCREEN_WIDTH / 2);
				double ly = double(ily) / (SCREEN_HEIGHT / 2);

				lx += (0.5 / SCREEN_WIDTH);
				ly += (0.5 / SCREEN_HEIGHT);
				lx = -lx;

//				printf("%f %f\n", lx, ly);
				
				ly = -ly;
				double dist = sqrt((lx * lx) + (ly * ly));
				dist /= sqrt(2.0);

				idist2 = int(dist * 255);
				idist2 &= 255;

				float deg = atan2(float(lx), float(ly));
				ideg2  = int(floor(deg * (255 / (2 * M_PI))));
				ideg2  &= 0xFF;
			}
			if (ideg2 != ideg1) printf("expected: %d, got: %d, err: %d\n", ideg2,  ideg1, ((ideg1 - ideg2) << 24) >> 24);
//			getchar();
/*			printf("dist: %d %d\n", idist1, idist2); */
		}
	}
	exit(0);
#endif

	printf("dumping offsetmapthingy...\n");
	{
		FILE *fp = fopen("zoom_table.bin", "wb");
		if (NULL != fp)
		{
			fwrite(zoom_table, 2, (SCREEN_WIDTH / 2) * SCREEN_HEIGHT, fp);
			fclose(fp);
		}
	}

	printf("warping image...\n");
	for (int y = 0; y < TEXTURE_HEIGHT; ++y)
	{
		for (int x = 0; x < 256; ++x)
		{

			int r = 0;
			int g = 0;
			int b = 0;
			for (int iy = 0; iy < SUPERSAMPLE; ++iy)
			{
				for (int ix = 0; ix < SUPERSAMPLE; ++ix)
				{
					double deg  = x + (double(ix) / SUPERSAMPLE);
					double dist = y + (double(iy) / SUPERSAMPLE);

					double rad = deg  * ((M_PI * 2) / 256);
					double d   = dist / 256;

					d /= (TEXTURE_HEIGHT / 256);
					d = pow(d, DIST_EXP);

					// new x and y
					double nx = sin(rad) * d;
					double ny = cos(rad) * d;

					nx *= sqrt(2.0);
					ny *= sqrt(2.0);

					nx = (nx + 1) * 0.5;
					ny = (ny + 1) * 0.5;

					RGBQUAD value = bilinear_sample(bmp, nx, ny);
					r += value.rgbRed;
					g += value.rgbGreen;
					b += value.rgbBlue;
				}
			}

			RGBQUAD value;
			value.rgbRed   = r / (SUPERSAMPLE * SUPERSAMPLE);
			value.rgbGreen = g / (SUPERSAMPLE * SUPERSAMPLE);
			value.rgbBlue  = b / (SUPERSAMPLE * SUPERSAMPLE);
			FreeImage_SetPixelColor(tex, x, TEXTURE_HEIGHT - y, &value);
		}
	}

	printf("storing warped image...\n");
	FreeImage_Save(FIF_PNG, tex, "zoom_map_24bpp.png");

/*	tex = FreeImage_Dither(tex, FID_BAYER8x8); */
	FIBITMAP *tex555 = FreeImage_ConvertTo16Bits555(tex);
	tex = FreeImage_ConvertTo24Bits(tex555);
	FIBITMAP *tex8   = FreeImage_ColorQuantizeEx(tex);

//	FreeImage_FlipVertical(tex8);
	FreeImage_Save(FIF_PNG, tex8, "zoom_map_8bpp.png");
	tex = FreeImage_ConvertTo24Bits(tex8);
	FreeImage_FlipVertical(tex);

	u16 zoom_lut[256];
	printf("dumping zoom lut...\n");
	{
		FILE *fp = fopen("zoom_lut.h", "w");
		if (NULL != fp)
		{
			fprintf(fp, "u32 zoom_lut[256] = {\n");
			for (int i = 0; i < 256; ++i)
			{
		//		float dist = pow((i * (1.0 / 256)), 1.75) * (256.0 / 1) * zoom;
				double dist = pow((i * (1.0 / 256)), 1.0 / DIST_EXP) * (256.0 / 1);
				u32 val = u32(dist * (1 << 8));
				zoom_lut[i] = val;
				ASSERT(zoom_lut[i] == val); /* check for over/underflow */
				fprintf(fp, "%d, ", zoom_lut[i]);
			}
			fprintf(fp, "\n};\n\n");
			fclose(fp);
		}
	}


	int swaps = 0;
	int start = GetTickCount();
	while (1)
	{
		double time = float(GetTickCount() - start) * (1.f / 1000);

		time *= 0.025;

//		double zoom = MIN(time, 1.0);

		double zoom = fmod(time, 1.0);
		zoom = pow(zoom, DIST_EXP);
		zoom *= double(TEXTURE_HEIGHT) / 256;
/*		
		double zoom = time;
		zoom *= double(TEXTURE_HEIGHT) / 256;
		zoom = fmod(zoom, pow(double(TEXTURE_HEIGHT) / 256, 1.0 / DIST_EXP));
		zoom = pow(zoom, DIST_EXP);
*/
//		zoom = 4.0 + (2.0 / 256);
//		zoom = 4.0;

		u32 dist_remap[256];
		for (int i = 0; i < 256; ++i)
		{
//			float dist = pow((i * (1.0 / 256)), 1.75) * (256.0 / 1) * zoom;
//			double dist = pow((i * (1.0 / 256)), 1.0 / DIST_EXP) * (256.0 / 1) * zoom * 256;
			u32 dist = zoom_lut[i] * zoom;
			dist_remap[i] = dist;
		}

		u8 *zt = zoom_table;
		for (int y = 0; y < SCREEN_HEIGHT; ++y)
		{
			for (int x = 0; x < SCREEN_WIDTH / 2; ++x)
			{
				int deg  = *zt++;
				int dist = dist_remap[*zt++];

				int dither_dist = dithermask[y & 3][x & 3];
				dist = (dist + dither_dist) >> 8;


				int uv = deg | dist << 8;
				ASSERT(uv < (256 * TEXTURE_HEIGHT));

				int col;
				RGBQUAD value;

				FreeImage_GetPixelColor(tex, uv & 0xFF, uv >> 8, &value);
				col = value.rgbBlue | (value.rgbGreen << 8) | (value.rgbRed << 16);

				fb[x + y * SCREEN_WIDTH] = col;

				uv ^= 0xFF;

				FreeImage_GetPixelColor(tex, uv & 0xFF, uv >> 8, &value);
				col = value.rgbBlue | (value.rgbGreen << 8) | (value.rgbRed << 16);

				fb[(SCREEN_WIDTH - x - 1) + y * SCREEN_WIDTH] = col;
			}
		}

/*		flip(fb, bb, pal); */
		ptc_update(fb);

		swaps++;
	}
}
