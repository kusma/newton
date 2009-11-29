#pragma once
#include <assert.h>
#include <FreeImage.h>

class Image
{
public:
	Image() : dib(NULL)
	{
	}
	
	Image(const Image &img)
	{
		dib = FreeImage_Clone(img.dib);
	}

	Image(FIBITMAP *dib) : dib(dib)
	{
	}
	
	~Image()
	{
		if (dib != NULL) FreeImage_Unload(dib);
	}
	
	bool create(int w, int h, int bpp)
	{
		dib = FreeImage_Allocate(w, h, bpp);
		return (dib != NULL);
	}
	
	void *getBits()
	{
		assert(dib != NULL);
		return FreeImage_GetBits(dib);
	}
	
	void *const getBits() const
	{
		assert(dib != NULL);
		return FreeImage_GetBits(dib);
	}
	
	const int getWidth() const
	{
		assert(dib != NULL);
		return FreeImage_GetWidth(dib);
	}
	
	const int getHeight() const
	{
		assert(dib != NULL);
		return FreeImage_GetHeight(dib);
	}
	
	const int getPitch() const
	{
		assert(dib != NULL);
		return FreeImage_GetPitch(dib);
	}
	
	const int getBPP() const
	{
		assert(dib != NULL);
		return FreeImage_GetBPP(dib);
	}
	
	RGBQUAD getPixelColor(int x, int y) const
	{
		assert(dib != NULL);
		assert(FIC_RGB == FreeImage_GetColorType(dib));

		RGBQUAD res;
		FreeImage_GetPixelColor(dib, x, y, &res);
		return res;
	}
	
	BYTE getPixelIndex(int x, int y) const
	{
		assert(dib != NULL);
		assert(FIC_PALETTE == FreeImage_GetColorType(dib));

		BYTE res;
		FreeImage_GetPixelIndex(dib, x, y, &res);
		return res;
	}

	bool setPixelIndex(int x, int y, BYTE color)
	{
		assert(dib != NULL);
		assert(FIC_PALETTE == FreeImage_GetColorType(dib));
		return FALSE == FreeImage_SetPixelIndex(dib, x, y, &color) ? false : true;
	}

	bool setPixelColor(int x, int y, RGBQUAD color)
	{
		assert(dib != NULL);
		assert(FIC_RGB == FreeImage_GetColorType(dib));
		return FALSE == FreeImage_SetPixelColor(dib, x, y, &color) ? false : true;
	}

	const RGBQUAD *const getPalette() const
	{
		assert(dib != NULL);
		assert(FIC_PALETTE == FreeImage_GetColorType(dib));

		return FreeImage_GetPalette(dib);
	}

	void setPalette(const RGBQUAD *const pal)
	{
		assert(dib != NULL);
		assert(FIC_PALETTE == FreeImage_GetColorType(dib));
		RGBQUAD *dst_pal = FreeImage_GetPalette(dib);
		memcpy(dst_pal, pal, sizeof(RGBQUAD) * 256);
	}


	unsigned getColorsUsed()
	{
		assert(dib != NULL);
		assert(FIC_PALETTE == FreeImage_GetColorType(dib));

		return FreeImage_GetColorsUsed(dib);
	}
	
	bool save(const char *filename) const
	{
		assert(dib != NULL);
		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename);
		FreeImage_FlipVertical(dib);
		return (TRUE == FreeImage_Save(fif, dib, filename));
	}

	bool load(const char *filename)
	{
		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename);
		dib = FreeImage_Load(fif, filename);
		if (dib == NULL) return false;
		
		FreeImage_FlipVertical(dib);

		return true;
	}

	const FIBITMAP *GetFIBitmap() const { return dib; }
	FIBITMAP *GetFIBitmap() { return dib; }

private:
	FIBITMAP *dib;
};
