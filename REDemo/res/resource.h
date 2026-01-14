//stamp:077208f72e4aa77e
/*<------------------------------------------------------------------------------------------------->*/
/*该文件由uiresbuilder生成，请不要手动修改*/
/*<------------------------------------------------------------------------------------------------->*/
#ifndef _UIRES_H_
#define _UIRES_H_
	struct _UIRES{
		struct _UIDEF{
			const TCHAR * XML_INIT;
			}UIDEF;
		struct _LAYOUT{
			const TCHAR * XML_MAINWND;
			}LAYOUT;
		struct _values{
			const TCHAR * string;
			const TCHAR * color;
			const TCHAR * skin;
			}values;
		struct _PNG{
			const TCHAR * IDB_PNG_LEFTBUBBLE;
			const TCHAR * IDB_PNG_RIGHTBUBBLE;
			const TCHAR * IDB_PNG_RIGHTOTHERBUBBLE;
			const TCHAR * IDB_PNG_DEFAULTBG;
			const TCHAR * IDB_PNG_RICHEDIT_MOREMSG;
			const TCHAR * IDB_PNG_RICHEDIT_MORELOADING;
			const TCHAR * IDB_PNG_RICHEDIT_RESEND;
			const TCHAR * ID_RICH_SYSMSG_BK;
			const TCHAR * IDB_PNG_BGSYSMSG;
			const TCHAR * IDB_PNG_REVOKE_REEDITOR;
			const TCHAR * ID_EMPTY;
			}PNG;
		struct _ICON{
			const TCHAR * ICON_LOGO;
			}ICON;
	};
#endif//_UIRES_H_
#ifdef INIT_R_DATA
struct _UIRES UIRES={
		{
			_T("UIDEF:XML_INIT"),
		},
		{
			_T("LAYOUT:XML_MAINWND"),
		},
		{
			_T("values:string"),
			_T("values:color"),
			_T("values:skin"),
		},
		{
			_T("PNG:IDB_PNG_LEFTBUBBLE"),
			_T("PNG:IDB_PNG_RIGHTBUBBLE"),
			_T("PNG:IDB_PNG_RIGHTOTHERBUBBLE"),
			_T("PNG:IDB_PNG_DEFAULTBG"),
			_T("PNG:IDB_PNG_RICHEDIT_MOREMSG"),
			_T("PNG:IDB_PNG_RICHEDIT_MORELOADING"),
			_T("PNG:IDB_PNG_RICHEDIT_RESEND"),
			_T("PNG:ID_RICH_SYSMSG_BK"),
			_T("PNG:IDB_PNG_BGSYSMSG"),
			_T("PNG:IDB_PNG_REVOKE_REEDITOR"),
			_T("PNG:ID_EMPTY"),
		},
		{
			_T("ICON:ICON_LOGO"),
		},
	};
#else
extern struct _UIRES UIRES;
#endif//INIT_R_DATA

#ifndef _R_H_
#define _R_H_
struct _R{
	struct _name{
		 const wchar_t * btn_close;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_restore;
		 const wchar_t * recv_richedit;
	}name;
	struct _id{
		int btn_close;
		int btn_max;
		int btn_min;
		int btn_restore;
		int recv_richedit;
	}id;
	struct _color{
		int blue;
		int gray;
		int green;
		int red;
		int white;
	}color;
	struct _string{
		int title;
		int ver;
	}string;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"btn_close",
		L"btn_max",
		L"btn_min",
		L"btn_restore",
		L"recv_richedit"
	}
	,
	{
		65536,
		65537,
		65539,
		65538,
		65540
	}
	,
	{
		0,
		1,
		2,
		3,
		4
	}
	,
	{
		0,
		1
	}
	
};
#else
extern struct _R R;
#endif//INIT_R_DATA
