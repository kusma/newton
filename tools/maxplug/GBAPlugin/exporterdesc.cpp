#include "saker.h"

#define GBASCENEEXPORTER_CLASS_ID	Class_ID(0x83375336^1521246, 0x38318654^9593785)

typedef int	 __declspec(dllimport) (*DOEXPORT_WRAP)(const TCHAR*, ExpInterface*, Interface*, BOOL, DWORD);

//#ifndef USE_BACKEND_DLL
//int __declspec(dllexport) DoExport(const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts, DWORD options);
//#endif

class SceneExport_wrapper : public SceneExport {
public:
	// Number of extemsions supported
	virtual int				ExtCount() {
		return 1;
	}
	// Extension #n (i.e. "3DS")
	virtual const TCHAR *	Ext(int n) {
		return _T("h");
	}
	// Long ASCII description (i.e. "Autodesk 3D Studio File")	
	virtual const TCHAR *	LongDesc() {
		return _T("GBA Scene export");
	}
	// Short ASCII description (i.e. "3D Studio")
	virtual const TCHAR *	ShortDesc() {
		return _T("GBA Scene");
	}
	// ASCII Author name
	virtual const TCHAR *	AuthorName() {
		return _T("demo coder");
	}
	// ASCII Copyright message
	virtual const TCHAR *	CopyrightMessage() {
		return _T("");
	}
	virtual const TCHAR *	OtherMessage1() {
		return _T("");
	}
	virtual const TCHAR *	OtherMessage2() {
		return _T("");
	}
	// Version number * 100 (i.e. v3.01 = 301)
	virtual unsigned int	Version() {
		return 001;
	}
	// Show DLL's "About..." box
	virtual void			ShowAbout(HWND hWnd) {
		return;
	}
	// Returns TRUE if all option bits set are supported for this extension
	virtual BOOL			SupportsOptions(int ext, DWORD options) {
		return TRUE ;
	}

	virtual int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0) {
		HMODULE hmodule = LoadLibrary("GbaExporterBackend.dll");
		if (!hmodule) {
			message("Backend DLL not loaded!");
			return IMPEXP_FAIL;
		}
		DOEXPORT_WRAP DoExport_wrap = (DOEXPORT_WRAP)GetProcAddress(hmodule, "DoExport");
		if (!DoExport_wrap) {
			message("Error in backend DLL: Entrypoint not found!");
			FreeLibrary(hmodule);
			return IMPEXP_FAIL;
		}
		int ret = DoExport_wrap(name, ei, i, suppressPrompts, options);
		FreeLibrary(hmodule);
		return ret;
	}

};



class ExporterClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new SceneExport_wrapper(); }
	const TCHAR *	ClassName() { return _T("GbaExporter"); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return GBASCENEEXPORTER_CLASS_ID; }
	const TCHAR* 	Category() { return _T("Sparks3d"); }
	const TCHAR*	InternalName() { return _T("GbaExporter"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				    // returns owning module handle
};



ClassDesc2* GetExporterDesc() { 
	static ExporterClassDesc tmp;
	return &tmp; 
}




