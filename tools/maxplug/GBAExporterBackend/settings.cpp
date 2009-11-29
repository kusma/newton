#include "settings.h"
#include "resource.h"
using namespace std;
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include <map>
#include <vector>
#include <algorithm>
#include <atlimage.h>
#include <IGame.h>
#include "max.h"
#include <string>
#include <iostream>

Settings::Settings() : antialias_masks(true) {

}

Settings global_settings;

void message(const char* msg) 
{
	MessageBox(NULL, msg, "GbaExporter Backend", 0);
}

// denne mapen oversetter fra shortname til fullname.
// Den brukes for å unngå kopiering av samme tekstur fler ganger,
// samt for å hindre navnekollisjoner
static std::map<std::string, std::string> texture_filename_conversions;

// denne registrerer errors
struct WarningData 
{
	WarningData() : fatal(false) {};
	std::vector<std::string> instances;
	bool fatal;
};
static std::map<std::string, WarningData> warning_collection;


std::string Settings::get_naked_path() 
{
	int slashpos1 = (int)filename.find_last_of("/");
	int slashpos2 = (int)filename.find_last_of("\\");
	int slashpos = max(slashpos1, slashpos2);
	if (slashpos>=0) 
	{
		return filename.substr(0, slashpos+1);
	} 
	else 
	{
		return "";
	}
}
std::string Settings::get_naked_file() 
{
	int slashpos1 = (int)filename.find_last_of("/");
	int slashpos2 = (int)filename.find_last_of("\\");
	int slashpos = max(slashpos1, slashpos2);
	if (slashpos>=0) 
	{
		return filename.substr(slashpos+1);
	}
	else 
	{
		return filename;
	}
}

static void multidel(std::string path, const char* mask) 
{
	WIN32_FIND_DATA finddata;
	std::string fullname = path+mask;
	HANDLE h = FindFirstFile(fullname.c_str(), &finddata);
	BOOL gogogo = (h!=INVALID_HANDLE_VALUE);
	while (gogogo) 
	{
		if (! (finddata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ) 
		{
			DeleteFile( (path+finddata.cFileName).c_str() );
		}
		gogogo = FindNextFile(h, &finddata);
	}
	FindClose(h);
}

void Settings::pre_export() {
	warning_collection.clear();
}

void Settings::post_export() {
	// vis alle warnings
	std::map<std::string, WarningData>::iterator iter;
	for (iter=warning_collection.begin(); iter!=warning_collection.end(); iter++) {
		WarningData& wd = iter->second;
		std::string txt;
		if (wd.fatal) txt += "FATAL ERROR:\n";
		else txt += "WARNING:\n";
		txt += iter->first;
		vector<std::string>& instances = wd.instances;
		if (instances.size()) {
			txt += "\n\n\n";
			vector<std::string>& instances = wd.instances;
			for (unsigned int i=0; i<instances.size(); i++) {
				txt += instances[i];
				txt += "\n";
			}
		}
		message(txt.c_str());
	}
}

void Settings::warning(const std::string& text, const std::string& instance) {
	vector<std::string>& instances = warning_collection[text].instances;
	for (unsigned int i=0; i<instances.size(); i++) {
		if (instances[i]==instance) return;
	}
	instances.push_back(instance);
}

void Settings::warning(const std::string& text) {
	warning_collection[text];
}


void Settings::init(string& filename) {
	char saker[512];
	strncpy(saker, filename.c_str(), 512);
	for (char* p=saker; *p; p++) {
		if (*p>='A' && *p<='Z') *p = int(*p) + 'a' - 'A'; 
	}
	this->filename = saker;
}




BOOL CALLBACK SettingsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static Settings* settings;

	char tempstr[512];
	Interface* core  = GetCOREInterface();
	TSTR maskefil;

	string path;
//	BOOL* lp = new BOOL;
	switch(message) {
		case WM_INITDIALOG:
//			SendMessage(GetDlgItem(hWnd, IDC_SLIDER2), TBM_SETRANGE, 1, MAKELONG(0, 100));
//			SendMessage(GetDlgItem(hWnd, IDC_SLIDER2), TBM_SETPOS, 1, 50);
			
//			SendMessage(GetDlgItem(hWnd, IDC_SLIDER3), TBM_SETRANGE, 1, MAKELONG(0, 100));
//			SendMessage(GetDlgItem(hWnd, IDC_SLIDER3), TBM_SETPOS, 1, 50);

			settings = (Settings*)lParam;
//			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam); 
//			CenterWindow(hWnd,GetParent(hWnd));
//			SetDlgItemText(hWnd, IDC_EDIT3, FALANX_GLOBAL_VERSION_FULLSTRING);
			SetDlgItemText(hWnd, IDC_EDIT_FILENAME2, settings->get_naked_file().c_str());
			SetDlgItemText(hWnd, IDC_EDIT_PATH, settings->get_naked_path().c_str());
//			SetDlgItemText(hWnd, IDC_EDIT_PATH_KEYFRAMES, (settings->get_naked_path() + "keyframes").c_str());
//			SetDlgItemText(hWnd, IDC_EDIT_PATH_MESHES, (settings->get_naked_path() + "meshes").c_str());
			//			SetDlgItemText(hWnd, IDC_EDIT_PATH_TEXTURES, (settings->get_naked_path() + "textures").c_str());
			SetDlgItemText(hWnd, IDC_EDIT_SUPERSAMPLING, "1.0");
			SetDlgItemText(hWnd, IDC_EDIT_LIGHTBIAS, "0.0");
			SetDlgItemText(hWnd, IDC_EDIT_FPS, "30.0");
			//SetDlgItemText(hWnd, IDC_EDIT_FIRST_TEXI, "0");
			CheckDlgButton(hWnd, IDC_CHECK_PRECALC, 1);
			CheckDlgButton(hWnd, IDC_CHECK_FORCE_OBJECT_SPACE, 0);

			core->GetRootNode()->GetUserPropString("maskefil", maskefil);
			SetDlgItemText(hWnd, IDC_EDIT_MASKEFIL, maskefil.data());


			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDOK:
//				settings->maxerrorpos		= SendMessage(GetDlgItem(hWnd, IDC_SLIDER2), TBM_GETPOS, 0, 0);
//				settings->maxerrorrot		= SendMessage(GetDlgItem(hWnd, IDC_SLIDER3), TBM_GETPOS, 0, 0);
//				settings->clean_textures	= (bool)IsDlgButtonChecked(hWnd, IDC_CHECK_CLEAN_TEXTURES);
//				settings->clean_meshes		= (bool)IsDlgButtonChecked(hWnd, IDC_CHECK_CLEAN_MESHES);
//				settings->clean_keyframes	= (bool)IsDlgButtonChecked(hWnd, IDC_CHECK_CLEAN_KEYFRAMES);
//				settings->skyrage			= (bool)IsDlgButtonChecked(hWnd, IDC_CHECK_SKYRAGE);
//				settings->hypnoglow			= (bool)IsDlgButtonChecked(hWnd, IDC_CHECK_HYPNO);
//				settings->export_keyframes	= (bool)IsDlgButtonChecked(hWnd, IDC_CHECK_KEYFRAMES);
//				settings->force_keyframes	= (bool)IsDlgButtonChecked(hWnd, IDC_CHECK_KEYFRAMES_FORCE);
				//settings->first_texi        = GetDlgItemInt(hWnd, IDC_EDIT_FIRST_TEXI, NULL, true);
				settings->fps			    = GetDlgItemFloat(hWnd, IDC_EDIT_FPS, NULL);
				settings->supersampling     = GetDlgItemFloat(hWnd, IDC_EDIT_SUPERSAMPLING, NULL);
				settings->lightbias         = GetDlgItemFloat(hWnd, IDC_EDIT_LIGHTBIAS, NULL);
				settings->precalc			= (bool)IsDlgButtonChecked(hWnd, IDC_CHECK_PRECALC);
				settings->force_object_space			= (bool)IsDlgButtonChecked(hWnd, IDC_CHECK_FORCE_OBJECT_SPACE);

				GetDlgItemText(hWnd, IDC_EDIT_MASKEFIL, tempstr, 512);
				core->GetRootNode()->SetUserPropString("maskefil", tempstr);
				settings->maskefil = tempstr;

				EndDialog(hWnd, 1);
				break;
			case IDCANCEL:
				EndDialog(hWnd,0);
				break;
			default:;
			}
			

		default:
			return FALSE;

	}
	return TRUE;

}
