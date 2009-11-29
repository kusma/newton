//
// TinyPTC by Gaffer
// www.gaffer.org/tinyptc
//
extern "C"
{
#include "tinyptc.h"
}

#define _USE_MATH_DEFINES
#include <math.h>
#include "base.h"

#include <windows.h>
#include <math.h>
#include <stdio.h>

#undef SCREEN_HEIGHT
#undef SCREEN_WIDTH

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

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

#define SUPERSAMPLE 4
#define SPRITE_SIZE 64
#define ANGLE_COUNT (128 * 2)

#include "sphere.h"
#include <limits>


void calc_sphere_table(u8 *dst, double angle)
{
	static int frame_count = 0;
	char temp[256];
	sprintf(temp, "sphere%02d.raw", frame_count++);

	FILE *fp = fopen(temp, "wb");
	if (NULL == fp) return;

	FILE *fp2 = fopen("offsets.raw", "wb");
	if (NULL == fp2) return;

	for (int y = 0; y < SPRITE_SIZE; ++y)
	{
		int empties = 0;
		for (int x = 0; x < SPRITE_SIZE / 2; ++x)
		{
			/* normalized integer coordinate */
			int ilx = x - SPRITE_SIZE / 2;
			int ily = y - SPRITE_SIZE / 2;

			/* un-normalized floating point coordinate */
			double rdx = double(ilx) / (SPRITE_SIZE / 2);
			double rdy = double(ily) / (SPRITE_SIZE / 2);
			/* make sure center is not inside first pixel, but rather between the four center ones */
			rdx += (0.5 / (SPRITE_SIZE / 2));
			rdy += (0.5 / (SPRITE_SIZE / 2));

			double rdz = 5.0;

			/* normalize vector */
			double mag = (rdx * rdx) + (rdy * rdy) + (rdz * rdz);
			mag = 1.0 / sqrt(mag);
			rdx *= mag;
			rdy *= mag;
			rdz *= mag;

#if 0
			Vector3 dst = ray.o - sphere.o;
			//	Vector3 dst = sphere.o - ray.o;
				fixed16 a = dot(dst, dst);
				fixed16 b = dot(dst, ray.d);
				fixed16 c = a - sphere.rr;
				fixed16 d = b * b - c;

				if (d < fixed16(0)) return 0;
			//	if (d < fixed16(0)) return -b - sqrt(-d);
				return -b - sqrt(d);
#else
			/* ray origin */
			double rox = 0.0;
			double roy = 0.0;
			double roz = -1.015 * 5;

			/* sphere origin */
			double sox = 0.0;
			double soy = 0.0;
			double soz = 0.0;

			/* ray.o - sphere.o  */
#if 1
			double dst_x = rox - sox;
			double dst_y = roy - soy;
			double dst_z = roz - soz;
#else
			double dst_x = sox - rox;
			double dst_y = soy - roy;
			double dst_z = soz - roz;
#endif


			/* dot(dst, dst) */
			double a = (dst_x * dst_x) + (dst_y * dst_y) + (dst_z * dst_z);

			/* dot(dst, ray.d) */
			double b = (dst_x * rdx) + (dst_y * rdy) + (dst_z * rdz);

			/* a - r^2 */
			double c = a - pow(1.0, 2.0);

			/* b^2 - c */
			double d = b * b - c;

			double t;
			if (d >= 0.0) t = -b - sqrt(d);
			else          t = std::numeric_limits<double>::infinity();
#endif

			if (t < std::numeric_limits<float>::infinity())
			{
				double hx = rox + rdx * t;
				double hy = roy + rdy * t;
				double hz = roz + rdz * t;

				hz -= soz;

				double mag = hx * hx + hy * hy + hz * hz;
				mag = 1.0 / sqrt(mag);
				hx *= mag;
				hy *= mag;
				hz *= mag;

				double nx = hx;
				double ny = cos(angle) * hy - sin(angle) * hz;
				double nz = sin(angle) * hy + cos(angle) * hz;

				double slong = atan2(nx, nz) / (2 * M_PI);
//				double slatt = ny / 2 + 0.5;
				double slatt = 1.0 - acos(ny) / M_PI;

				int iu = floor(slong * 256);
				int iv = int(slatt * 256);

/*				hx /= hz;
				hy /= hz; */
/*
				int iu = 128 + int(hx * 128);
				int iv = 128 + int(hy * 128);
*/
				/* store in LUT */
				dst[(x + y * (SPRITE_SIZE / 2)) * 2 + 0] = iu;
				dst[(x + y * (SPRITE_SIZE / 2)) * 2 + 1] = iv;
				fwrite(&dst[(x + y * (SPRITE_SIZE / 2)) * 2 + 0], 1, 1, fp);
				fwrite(&dst[(x + y * (SPRITE_SIZE / 2)) * 2 + 1], 1, 1, fp);
			}
			else
			{
				/* store in LUT */
				dst[(x + y * (SPRITE_SIZE / 2)) * 2 + 0] = 0;
				dst[(x + y * (SPRITE_SIZE / 2)) * 2 + 1] = 0;
				empties++;
			}
		}
		fwrite(&empties, 4, 1, fp2);
//		fwrite(&empties, 4, 1, fp2);
	}
	fclose(fp);
	fclose(fp2);
}

int main()
{
	/*	printf("%f\n", floor(0.9 + 0.5)); */
	if (!ptc_open("test", SCREEN_WIDTH, SCREEN_HEIGHT)) return 1;

#if 1
	Image tex;
	if (!tex.Load("image.png"))
	{
		printf("error: failed to load image\n");
		exit(1);
	}
#endif
/*
	FIBITMAP *tex = FreeImage_Allocate(256, TEXTURE_HEIGHT, 24);
	if (NULL == tex)
	{
		printf("error: failed to create image\n");
		exit(1);
	}
*/

	/* setup zoom-table */
	static u8 zoom_tables[ANGLE_COUNT / 4 + 1][(SPRITE_SIZE / 2) * SPRITE_SIZE * 2];
	for (int i = 0; i < ANGLE_COUNT / 4 + 1; ++i)
	{
		calc_sphere_table(zoom_tables[i], i * ((2 * M_PI) / ANGLE_COUNT));
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

#if 0
	printf("dumping offsetmapthingy...\n");
	{
		FILE *fp = fopen("zoom_table.bin", "wb");
		if (NULL != fp)
		{
			fwrite(zoom_tables, 2, (SCREEN_WIDTH / 2) * SCREEN_HEIGHT, fp);
			fclose(fp);
		}
	}
#endif

#if 0
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
#endif

/*	tex = FreeImage_Dither(tex, FID_BAYER8x8); */

/*	FIBITMAP *tex555 = FreeImage_ConvertTo16Bits555(tex.GetFIBitmap());
	tex = FreeImage_ConvertTo24Bits(tex555); */
	FIBITMAP *tex8   = FreeImage_ColorQuantizeEx(tex.GetFIBitmap());

//	FreeImage_FlipVertical(tex8);
/*	FreeImage_Save(FIF_PNG, tex8, "zoom_map_8bpp.png");
	tex = FreeImage_ConvertTo24Bits(tex8);
	FreeImage_FlipVertical(tex);
*/
#if 0
	printf("dumping zoom lut...\n");
	{
		FILE *fp = fopen("zoom_lut.h", "w");
		if (NULL != fp)
		{
			fprintf(fp, "u32 zoom_lut[256] = {\n");
			u32 dist_remap[256];
			for (int i = 0; i < 256; ++i)
			{
		//		float dist = pow((i * (1.0 / 256)), 1.75) * (256.0 / 1) * zoom;
				double dist = pow((i * (1.0 / 256)), 1.0 / DIST_EXP) * (256.0 / 1);
				dist_remap[i] = u32(dist * (1 << 8));
				fprintf(fp, "%d, ", dist_remap[i]);
			}
			fprintf(fp, "\n};\n\n");
			fclose(fp);
		}
	}
#endif

	printf("axxion!\n");
	int swaps = 0;
	int start = GetTickCount();
	while (1)
	{
		double time = float(GetTickCount() - start) * (1.f / 1000);

//		double zoom = MIN(time, 1.0);

		double zoom = fmod(time, 1.0);
/*		zoom = pow(zoom, DIST_EXP);
		zoom *= double(TEXTURE_HEIGHT) / 256;
*/
/*		
		double zoom = time;
		zoom *= double(TEXTURE_HEIGHT) / 256;
		zoom = fmod(zoom, pow(double(TEXTURE_HEIGHT) / 256, 1.0 / DIST_EXP));
		zoom = pow(zoom, DIST_EXP);
*/
//		zoom = 4.0 + (2.0 / 256);
//		zoom = 4.0;

		int slong_offs = time * 100;
		float xrotf = time * 0.1f;

		int xrot = int(xrotf * ANGLE_COUNT);

		/* detect first flip */
		int flip1 = (xrot & (ANGLE_COUNT / 2)) != 0;
		xrot &= (ANGLE_COUNT / 2) - 1;

		/* detect second flip */
		int flip2 = (xrot & ANGLE_COUNT / 4) != 0;
		xrot &= (ANGLE_COUNT / 4) - 1;

		/* calculate xor masks */
		int slong_mask = 0;
		if (0 != (flip1 ^ flip2)) slong_mask = 255;

		int slatt_mask = 0;
		if (0 != flip1)
		{
			slatt_mask = 255;
			slong_offs += 128;
		}

		if (0 != flip2)
		{
			/* TODO: avoid a gap when flip2 goes high by making the LUT one element bigger, and changing the subtraction below to ANGLE_COUNT / 4 - xrot */
			xrot = ANGLE_COUNT / 4 - xrot;
			slong_offs += 128; 
		}

		printf("%d %d\n", flip1, flip2);

		/* setup ymask */
		int ymask = 0;
		if (0 != flip2) ymask = SPRITE_SIZE - 1;

		int xor_mask = slong_mask | (slatt_mask << 8);
		xor_mask = ymask | (xor_mask << 8);

		u8 *frame = zoom_tables[xrot];
		for (int y = 0; y < SPRITE_SIZE; ++y)
		{
//			int yline = y; // ^ (xor_mask & (SPRITE_SIZE - 1));
			int yline = (xor_mask & (SPRITE_SIZE - 1)) == 0 ? y : 64 - y;
			u16 *zt = (u16*)&frame[yline * SPRITE_SIZE];
			for (int x = 0; x < SPRITE_SIZE / 2; ++x)
			{
				int col;


/* address:
YYYXXXyyyxxx
*/

				int ztval = zt[x];
//				int ztval = *zt++;
				ztval ^= (xor_mask >> 8); /* yez, xor mask is constant! */

				int slong = ztval & 0xFF;
				int slatt = ztval >> 8;

				int lslong = slong + slong_offs;
				lslong &= 0xFF;

				int uv = lslong | slatt << 8;
				RGBQUAD value = sample(tex, uv & 0xFF, uv >> 8);
				col = value.rgbBlue | (value.rgbGreen << 8) | (value.rgbRed << 16);
				fb[x + y * SCREEN_WIDTH] = col;

				int rslong = slong ^ 0xFF;
				rslong += slong_offs;
				rslong &= 0xFF;

				uv = rslong | slatt << 8;
				value = sample(tex, uv & 0xFF, uv >> 8);
				col = value.rgbBlue | (value.rgbGreen << 8) | (value.rgbRed << 16);
				fb[y * SCREEN_WIDTH + SPRITE_SIZE - 1 - x] = col;
			}
		}

/*		flip(fb, bb, pal); */
		ptc_update(fb);

		swaps++;
	}
}
