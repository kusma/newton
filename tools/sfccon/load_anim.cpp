#include <windows.h>								// Header File For Windows
#include <Vfw.h>
#include "main.h"
#include "load_anim.h"


/**
FreeImage error handler
@param fif Format / Plugin responsible for the error
@param message Error message
*/
static bool someError;
static void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
	fprintf(stderr, "%s\n", message);
	someError=true;
}

static HDC hdc = CreateCompatibleDC(0);								// Creates A Compatible Device Context

static FIMULTIBITMAP* ReadFromAvi(const char* filename) {
	int err=0;
	AVIFileInit();
	
	PAVISTREAM pavi; // Handle To An Open Stream
	if( AVIStreamOpenFromFile(&pavi, filename, streamtypeVIDEO, 0, OF_READ, NULL) != 0) {
		AVIFileExit();
		return NULL;
	}



	AVISTREAMINFO		psi;				// Pointer To A Structure Containing Stream Info
	AVIStreamInfo(pavi, &psi, sizeof(psi));				// Reads Information About The Stream Into psi
	int width  = psi.rcFrame.right-psi.rcFrame.left;			// Width Is Right Side Of Frame Minus Left
	int height = psi.rcFrame.bottom-psi.rcFrame.top;			// Height Is Bottom Of Frame Minus Top
	int frameCount = AVIStreamLength(pavi);							// The Last Frame Of The Stream

	double mpf = AVIStreamSampleToTime(pavi, frameCount) / (double)frameCount;		// Calculate Rough Milliseconds Per Frame

	PGETFRAME pgf = AVIStreamGetFrameOpen(pavi, NULL);				// Create The PGETFRAME Using Our Request Mode
	if (pgf==NULL)
	{
		// An Error Occurred Opening The Frame
		error("Failed To Open frame from AVI");
	}

	//HDC hdc = GetDC(0);

	HDRAWDIB hdd = DrawDibOpen();													// Handle For Our Dib

	BITMAPINFOHEADER bmih;										// Header Information For DrawDibDraw Decoding
	bmih.biSize = sizeof (BITMAPINFOHEADER);					// Size Of The BitmapInfoHeader
	bmih.biPlanes = 1;											// Bitplanes	
	bmih.biBitCount = 24;										// Bits Format We Want (24 Bit, 3 Bytes)
	bmih.biWidth = width;										// Width We Want (256 Pixels)
	bmih.biHeight = height;										// Height We Want (256 Pixels)
	bmih.biCompression = BI_RGB;								// Requested Mode = RGB

	char		*data;						// Pointer To Texture Data
	HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO*)(&bmih), DIB_RGB_COLORS, (void**)(&data), NULL, NULL);
	SelectObject(hdc, hBitmap);								// Select hBitmap Into Our Device Context (hdc)

	// create a new freeimage anim
	someError=false;
	FIMULTIBITMAP* ret = FreeImage_OpenMultiBitmap(FIF_TIFF, "temp.tiff", TRUE, FALSE);
	if (!ret || someError) {
		error("Couldnt create multibitmap");
	}

	for (int frame=0; frame<frameCount; frame++) {
		fprintf(stderr, "Loading frame %i\n", frame);
		// Grab Data From The AVI Stream
		LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, frame);	
		// Pointer To Data Returned By AVIStreamGetFrame
		// (Skip The Header Info To Get To The Data)
		char* pdata = (char *)lpbi + lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD);	
		
		// Convert Data To Requested Bitmap Format
		DrawDibDraw(hdd, hdc, 0, 0, width, height, lpbi, pdata, 0, 0, width, height, 0);

		// copy into the freeimage thing
		FIBITMAP* fiBitmap = FreeImage_ConvertFromRawBits((BYTE*)data, width, height, width*3, 24, 0xFF0000, 0x00FF00, 0x0000FF);
/*		BYTE* src = (BYTE*)data;
		for (int y=0; y<height; y++) {
			BYTE* dst = FreeImage_GetScanLine(fiBitmap, y);
			for (int x=0; x<width; x++) {
				//src++;
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = *src++;
			}
		}
*/
		FIBITMAP* grayBitmap = FreeImage_ConvertToGreyscale(fiBitmap);
		FreeImage_Unload(fiBitmap);

		FreeImage_AppendPage(ret, grayBitmap);
	}
	FreeImage_CloseMultiBitmap(ret);
	ret = FreeImage_OpenMultiBitmap(FIF_TIFF, "temp.tiff", FALSE, TRUE);

	DeleteObject(hBitmap);										// Delete The Device Dependant Bitmap Object
	DrawDibClose(hdd);											// Closes The DrawDib Device Context
	AVIStreamGetFrameClose(pgf);								// Deallocates The GetFrame Resources
	AVIStreamRelease(pavi);										// Release The Stream
	AVIFileExit();												// Release The File

	return ret;
}

FIMULTIBITMAP* LoadAnim(const char* filename) {
	FreeImage_SetOutputMessage(FreeImageErrorHandler);

	FIMULTIBITMAP* ret(NULL);
	
	// try animated GIF
	{
		someError=false;
		ret = FreeImage_OpenMultiBitmap(FIF_GIF, filename, FALSE, TRUE);
		if (ret && !someError) {
			fprintf(stderr, "Loaded animated GIF..\n");
			return ret;
		}
	}
	
	// try AVI
	{
		ret = ReadFromAvi(filename);
		if (ret) {
			fprintf(stderr, "Loaded AVI..\n");
			return ret;
		}
	}

	return NULL;
}
