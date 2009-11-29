#pragma once
#define WINDOWS_LEAN_AND_MEAN
#include "windows.h"
#include <string>

struct Settings {
	Settings();
	void init(std::string& filename);
	std::string get_filename() { return filename; }
	std::string get_naked_path();
	std::string get_naked_file();
	int			get_frametime() {
		return int(4800/fps); // 160 gir 30 fps
		//return fps*128/25;
	}

	bool force_object_space;
	float supersampling;
	bool precalc;
	float  fps;
	float lightbias;
	std::string maskefil;
	bool antialias_masks;



	// tools
	void		warning(const std::string& text, const std::string& instance);
	void		warning(const std::string& text);
	void		pre_export();
	void		post_export();
	//std::string copy_texture(std::string fullname);
private:
	std::string filename;
};

BOOL CALLBACK SettingsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void message(const char* msg);
extern Settings global_settings;
