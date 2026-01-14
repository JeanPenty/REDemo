// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#define  _CRT_SECURE_NO_WARNINGS
#define	 DLL_SOUI
#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/SouiCtrls.h>
#include <res.mgr/SObjDefAttr.h>
#include <com-cfg.h>
#include <SouiFactory.h>
#include "resource.h"
#include <layout/SouiLayoutParamStruct.h>
#include <atl.mini/SComCli.h>
using namespace SNS;

#include <Imm.h>
#pragma comment(lib, "imm32.lib")

#include <string>
#include <IPHlpApi.h>
#pragma comment(lib, "Iphlpapi.lib") 
#include <MMSystem.h>
#pragma comment(lib,"Winmm.lib")
#include <list>
#include <algorithm>
#include <iostream>
#include <Mshtml.h>
#include <atlconv.h>
using namespace std;

#include <sstream>
#include <fstream>
#include <math.h>
#include <io.h>  
#include <Shlobj.h>  
#pragma comment(lib, "shell32.lib")

#include <GdiPlus.h>
using namespace Gdiplus;
#pragma comment(lib, "Gdiplus.lib")

#include <Shlwapi.h>

#include "RichEditObjEvents.h"
#include "SImRichEdit.h"