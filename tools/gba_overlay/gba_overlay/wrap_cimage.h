#include <assert.h>
#include <FreeImage.h>

class CImage
{
public:
	CImage() : dib(NULL)
	{
	}

	CImage(FIBITMAP *dib) : dib(dib)
	{
	}

	~CImage()
	{
		if (dib != NULL) FreeImage_Unload(dib);
	}

	bool Create(int w, int h, int bpp)
	{
		dib = FreeImage_Allocate(w, h, bpp);
		return (dib != NULL);
	}

	void *GetBits()
	{
		assert(dib != NULL);
		return FreeImage_GetBits(dib);
	}

	const int GetWidth() const
	{
		assert(dib != NULL);
		return FreeImage_GetWidth(dib);
	}

	const int GetHeight() const
	{
		assert(dib != NULL);
		return FreeImage_GetHeight(dib);
	}

	const int GetPitch() const
	{
		assert(dib != NULL);
		return FreeImage_GetPitch(dib);
	}

	bool Save(const char *filename)
	{
		bool ret;
		assert(dib != NULL);
		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename);
		FreeImage_FlipVertical(dib);
		ret = (TRUE == FreeImage_Save(fif, dib, filename));
		FreeImage_FlipVertical(dib);
		return ret;
	}

	bool Load(const char *filename)
	{
		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename);
		FIBITMAP *temp = FreeImage_Load(fif, filename);
		if (temp == NULL) return false;

		colors = FreeImage_GetColorsUsed(temp);
		colors = 1 << FreeImage_GetBPP(temp);

		dib = FreeImage_ConvertTo32Bits(temp);
		FreeImage_Unload(temp);
		if (dib == NULL) return false;
		
		FreeImage_FlipVertical(dib);

		return true;
	}

	FIBITMAP *GetFIBitmap() { return dib; }

	int colors;
private:
	FIBITMAP *dib;
};
