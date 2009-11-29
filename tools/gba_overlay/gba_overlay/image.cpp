/*

  Routine for loading JPEG, GIF and BMP images through win32
  Copyright 2003 Erik Faye-Lund aka kusma/excess

*/

#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <ocidl.h>
#include <olectl.h>
#include <crtdbg.h>
#define HIMETRIC_INCH	2540

bool image_load(char *filename, unsigned **pixels, unsigned *w, unsigned *h)
{
	BITMAPINFO bi;
	HGLOBAL global;
	void* temp_data;
	LPSTREAM stream;
	HRESULT hr;
	LPPICTURE picture;
	long width, height;
	int width_, height_;

	HBITMAP bitmap;
	HDC hdc, hdc_mem;
	unsigned int* image_data;

	FILE *fp = fopen(filename, "rb");
	if (!fp) return FALSE;

	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);
	rewind(fp);

	if (w) *w = 0;
	if (h) *h = 0;
	*pixels = NULL;

	global = GlobalAlloc(GMEM_MOVEABLE, file_size);
	temp_data = GlobalLock( global );
	fread(temp_data, 1, file_size, fp);
//	memcpy(temp_data, file->data, file->size);

	GlobalUnlock(global );
	stream = NULL;
	hr = CreateStreamOnHGlobal( global, TRUE, &stream );
	if(FAILED(hr)) return FALSE;

	OleLoadPicture(
		stream,
		LONG(file_size),
		FALSE,
		IID_IPicture,
		(LPVOID *)&picture
	);

	stream->Release();
	fclose(fp);

	if (!picture) return FALSE;
	
	picture->get_Width(&width );
	picture->get_Height(&height );

	hdc = GetDC( NULL );
	width_ = MulDiv(width, GetDeviceCaps(hdc, LOGPIXELSX), HIMETRIC_INCH);
	height_ = MulDiv(height, GetDeviceCaps(hdc, LOGPIXELSY), HIMETRIC_INCH);

	bitmap = CreateCompatibleBitmap(hdc, width_, height_ );
	hdc_mem = CreateCompatibleDC(hdc);
	SelectObject(hdc_mem, bitmap);

	picture->Render(hdc_mem, 0, 0, width_, height_, 0, height, width, -height, NULL);
	picture->Release();
	image_data = (unsigned *) malloc(sizeof(unsigned) * width_ * height_);

	memset( &bi, 0, sizeof(BITMAPINFO) );
	bi.bmiHeader.biSize=sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = width_;
	bi.bmiHeader.biHeight = -height_;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	GetDIBits( hdc_mem, bitmap, 0, height_, image_data, &bi, DIB_RGB_COLORS );
	DeleteDC(hdc_mem);
	DeleteObject(bitmap);

	if (w) *w = width_;
	if (h) *h = height_;
	*pixels = image_data;
	return TRUE;
}