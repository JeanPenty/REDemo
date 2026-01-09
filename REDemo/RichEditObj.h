// ------------------------------------------------------------------------------
//
// RichEditObj.h : interface of the RichEditObj class
//
// SImRichEdit对象的基类头文件定义，SImRichEdit对象包括但不仅限于以下对象
// 
// - 气泡, 非OLE对象
// - 头像, 非OLE对象
// - 文本, 非OLE对象
// - 图片, OLE对象
// - 文件消息, OLE对象
// - 以上是历史消息, OLE对象
// - 获取更多, OLE对象
//
// 该文件只定义了非OLE对象的RichEdit对象，OLE对象的定义见 RichEditOleCtrls.h
// 
// ------------------------------------------------------------------------------

#pragma once
#include <vector>
#include <TOM.h>
#include "helper/obj-ref-impl.hpp"
#include "res.mgr/SFontPool.h"
#include "interface/SRender-i.h"
#include <atlcomcli.h>
#include "IRichEditObjHost.h"
namespace SOUI
{
#define REOBJ_FIRST   ((RichEditObj*)-1)    /*子对象插入在开头*/
#define REOBJ_LAST    NULL                  /*子对象插入在末尾*/
#define DECLARE_REOBJ(obj,name) \
    DEF_SOBJECT(obj,name) \
    static RichEditObj* CreateObject(){ return new obj; }

	class RichEditObj : public SObject
	{
		DEF_SOBJECT(SObject, L"re_obj");
	public:
		enum AlignType
		{
			ALIGN_LEFT,
			ALIGN_CENTER,
			ALIGN_RIGHT
		};

		RichEditObj();
		virtual ~RichEditObj();

		LONG AddRef();
		LONG Release();

		virtual BOOL InitFromXml(IXmlNode* pNode);
		virtual void DrawObject(IRenderTarget*);
		virtual LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual BOOL OnUpdateToolTip(CPoint pt, SwndToolTipInfo& tipInfo);
		virtual BOOL InsertIntoHost(IRichEditObjHost* pHost);
		virtual void UpdatePosition();
		virtual void SetDirty(BOOL);
		virtual void OffsetCharRange(int nOffset, BOOL bUpdate = FALSE);
		virtual void ExpandCharRange(int startCp, int nOffset, BOOL bUpdate = FALSE);
		virtual BOOL NeedToProcessMessage() { return FALSE; }
		virtual BOOL GetHitTestable() { return TRUE; }
		virtual AlignType GetAlign() { return _alignType; }
		virtual void SetAlign(AlignType align);
		virtual CRect GetRect() { return _objRect; }
		virtual BOOL PointInObject(POINT pt) { return _objRect.PtInRect(pt); }
		virtual SStringW GetObjType() { return GetClassName(); }
		CHARRANGE GetCharRange() { return _contentChr; }
		void AdjustMessageParam(UINT msg, WPARAM& wParam, LPARAM& lParam);

		SStringW GetName() { return _objName; }
		SStringW GetId() { return _objId; }
		SStringW GetData() { return _userData; }
		int GetScale() const { return 100; }

		// 树操作
		UINT GetChildrenCount();
		void InsertChild(RichEditObj* pNewChild, RichEditObj* pInsertAfter = REOBJ_LAST);
		BOOL RemoveChild(RichEditObj* pChild);
		BOOL CreateChildren(IXmlNode* pNode);
		RichEditObj* GetById(LPCWSTR lpszId);
		RichEditObj* GetByName(LPCWSTR lpszName);
		RichEditObj* FindChildByName(LPCWSTR pszName, int nDeep = -1);
		RichEditObj* GetParent()const { return _pParent; }
		RichEditObj* GetNext()const { return _pNextSibling; }
		RichEditObj* GetPrev()const { return _pPrevSibling; }
		RichEditObj* GetLastChild()const { return _pLastChild; }
		RichEditObj* GetFirstChild()const { return _pFirstChild; }
		void DestroyObject();
		BOOL DestroyChild(RichEditObj* pChild);
		void OnDestroy();

	protected:
		SOUI_ATTRS_BEGIN()
			ATTR_RECT(L"margin", _marginRect, FALSE)
			ATTR_STRINGW(L"id", _objId, FALSE)
			ATTR_STRINGW(L"name", _objName, FALSE)
			ATTR_STRINGW(L"cursor", _cursorName, FALSE)
			ATTR_STRINGW(L"data", _userData, FALSE)
			ATTR_ENUM_BEGIN(L"align", AlignType, FALSE)
			ATTR_ENUM_VALUE(L"left", ALIGN_LEFT)
			ATTR_ENUM_VALUE(L"center", ALIGN_CENTER)
			ATTR_ENUM_VALUE(L"right", ALIGN_RIGHT)
			ATTR_ENUM_END(_alignType)
			SOUI_ATTRS_END()

		RichEditObj* _pParent;       // 父节点 
		RichEditObj* _pFirstChild;   // 第一子节点 
		RichEditObj* _pLastChild;    // 最后节点 
		RichEditObj* _pNextSibling;  // 前一兄弟节点 
		RichEditObj* _pPrevSibling;  // 后一兄弟节点 
		UINT _childrenCount; // 子节点数量 
		LONG _references;    // 引用计数器
		SStringW _objId;         // 对象ID
		SStringW _objName;       // 对象名称 
		SStringW _userData;      // 用户数据
		SStringW _cursorName;    // 光标名称 
		IRichEditObjHost* _pObjHost;      // 宿主richedit 
		CHARRANGE _contentChr;    // 在richedit里面的字符下标,这个信息很重要
		CRect _marginRect;    // 对象的外边距 
		CRect _objRect;       // 在richedit里面的位置 
		BOOL _isDirty;       // 位置信息改变了 
		AlignType _alignType;     // 对齐方式,特指横向的对齐方式
	};


	class RichEditContent : public RichEditObj
	{
#define THRESHOLD_FOR_AUTOLAYOUT 2400 // 自动布局的阀值
		DEF_SOBJECT(RichEditObj, L"RichEditContent")

	public:
		RichEditContent() :_autoLayout(FALSE) {}
		~RichEditContent() {}

		virtual void UpdatePosition();
		virtual BOOL GetHitTestable() { return FALSE; }
		virtual CRect GetRect();
		SStringW GetObjType() { return GetClassName(); }
		SStringW GetContentType() { return _contentType; }
		time_t GetContentTimestamp() { return _timestamp; }
		SStringW GetContentTalkType() { return _contentTalkType; }
		SStringW GetContentOrigin() { return _contentOrigin; }
		SStringW GetContentMsgType() { return _contentMsgType; }
		SStringW GetContnetID() { return _contentID; }
	protected:
		BOOL OnTimestampAttr(const SStringW& attr, BOOL bLoading);
		SOUI_ATTRS_BEGIN()
			ATTR_STRINGW(L"type", _contentType, FALSE)
			ATTR_INT(L"auto-layout", _autoLayout, TRUE) /**< 是否自动布局,该设置会覆盖align属性*/
			ATTR_CUSTOM(L"timestamp", OnTimestampAttr)	//该条消息的时间戳
			ATTR_STRINGW(L"talk_type", _contentTalkType, FALSE)	//聊天类型
			ATTR_STRINGW(L"origin", _contentOrigin, FALSE)		//消息id
			ATTR_STRINGW(L"msgtype", _contentMsgType, FALSE)
			ATTR_STRINGW(L"content_id", _contentID, FALSE)
		SOUI_ATTRS_END()

	protected:
		SStringW    _contentType;
		BOOL        _autoLayout;
		time_t      _timestamp;
		SStringW	_contentTalkType;
		SStringW	_contentOrigin;
		SStringW	_contentMsgType;
		SStringW	_contentID;
	};
}



