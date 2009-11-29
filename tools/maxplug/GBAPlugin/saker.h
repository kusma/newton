#pragma once

#define WINDOWS_LEAN_AND_MEAN
#include "windows.h"
#include <IGame.h>
#include "Max.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

ClassDesc2* GetExporterDesc();
ClassDesc2* GetMaterialDesc();
extern HINSTANCE hInstance;
void message(char* msg);
