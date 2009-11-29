#pragma once

class Bitmap
{
public:
	Bitmap() : dib(NULL) {}

	~Bitmap()
	{
		if (NULL != dib)
		{
			FreeImage_Unload(dib);
			dib = NULL;
		}
	}

	explicit Bitmap(FIBITMAP *dib) : dib(dib) {}

	operator FIBITMAP*()
	{
		return dib;
	}

	operator FIBITMAP * const() const
	{
		return dib;
	}

protected:
	FIBITMAP *dib;
};

inline Bitmap load_bitmap(const char *filename)
{
	
}
