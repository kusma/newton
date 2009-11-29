#include "saker.h"	

HINSTANCE hInstance;

void message(char* msg) {
	MessageBox(NULL, msg, "GbaPlugin Frontend", 0);
}



// entry point
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;				// Hang on to this DLL's instance handle.
	return (TRUE);
}


__declspec( dllexport ) const TCHAR* LibDescription()
{
	return _T("GbaPlugin Frontend");
}


//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
	return 2;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetExporterDesc();
		case 1: return GetMaterialDesc();
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}
