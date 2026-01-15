#pragma once

#include "RichEditObj.h"
#include "ClipboardConverter.h"



namespace SOUI
{
	//----------------------------------------------------------------------------------
	//
	// RichEdit 相关通知，起始偏移900
	//
	//----------------------------------------------------------------------------------
#define EVT_RICHEDIT_BEGIN              (EVT_EXTERNAL_BEGIN + 900)
#define EVT_RE_QUERY_ACCEPT             (EVT_RICHEDIT_BEGIN+0)
#define EVT_RE_OBJ                      (EVT_RICHEDIT_BEGIN+1)
#define EVT_RE_SCROLLBAR                (EVT_RICHEDIT_BEGIN+2)

	class RichFormatConv;
	class RichEditObj;
	DEF_EVT_EXT(EventQueryAccept, EVT_RE_QUERY_ACCEPT, {
	RichFormatConv * Conv;
		});

	DEF_EVT_EXT(EventRichEditObj, EVT_RE_OBJ, {
		RichEditObj * RichObj;
		int           SubEventId;
		WPARAM        wParam;
		LPARAM        lParam;

		});

	DEF_EVT_EXT(EventRichEditScroll, EVT_RE_SCROLLBAR, {
	int WheelDelta;
	BOOL ScrollAtTop;
	BOOL ScrollAtBottom;

		});

	//----------------------------------------------------------------------------------
	//
	// ImgCache 相关通知，起始偏移1000
	//
	//----------------------------------------------------------------------------------

#define EVT_IMGCACHE_BEGIN          (EVT_EXTERNAL_BEGIN + 1000)
#define EVT_DONE_UPDATE             (EVT_IMGCACHE_BEGIN+0)

	class ImageAttr;

	DEF_EVT_EXT(EventImgCacheNotify, EVT_DONE_UPDATE, {
		std::vector<ImageAttr*>* Attrs;
		int         Context;

		});
}

