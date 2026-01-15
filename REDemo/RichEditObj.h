// ------------------------------------------------------------------------------
//
// RichEditObj.h : interface of the RichEditObj class
//
// SImRichEdit����Ļ���ͷ�ļ����壬SImRichEdit��������������������¶���
// 
// - ����, ��OLE����
// - ͷ��, ��OLE����
// - �ı�, ��OLE����
// - ͼƬ, OLE����
// - �ļ���Ϣ, OLE����
// - ��������ʷ��Ϣ, OLE����
// - ��ȡ����, OLE����
//
// ���ļ�ֻ�����˷�OLE�����RichEdit����OLE����Ķ���� RichEditOleCtrls.h
// 
// ------------------------------------------------------------------------------

#pragma once
#include <vector>
#include <TOM.h>
#include "souistd.h"
#include "helper/obj-ref-impl.hpp"
#include "res.mgr/SFontPool.h"
#include "interface/SRender-i.h"
#include <atlcomcli.h>
#include "IRichEditObjHost.h"
#include "RichEditUintConverter.h"
#include "RichEditObjFactory.h"

namespace SOUI
{
#define REOBJ_FIRST   ((RichEditObj*)-1)   
#define REOBJ_LAST    NULL                 
// DECLARE_REOBJ - 替换为直接定义 DEF_SOBJECT 和 CreateObject
#define DECLARE_REOBJ(obj,name) \
    DEF_SOBJECT(obj,name) \
    public: static RichEditObj* CreateObject() { return new obj(); } private:

#pragma region RichEditObj
	class RichEditObj : public SObject
	{
		DEF_SOBJECT(RichEditObj, L"re_obj");
	public:
		enum AlignType
		{
			ALIGN_LEFT,
			ALIGN_CENTER,
			ALIGN_RIGHT
		};

		RichEditObj();
		virtual ~RichEditObj();

		virtual void OnFinalRelease() {}

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

		// ������
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

		RichEditObj* _pParent;       // ���ڵ� 
		RichEditObj* _pFirstChild;   // ��һ�ӽڵ� 
		RichEditObj* _pLastChild;    // ���ڵ� 
		RichEditObj* _pNextSibling;  // ǰһ�ֵܽڵ� 
		RichEditObj* _pPrevSibling;  // ��һ�ֵܽڵ� 
		UINT _childrenCount; // �ӽڵ����� 
		LONG _references;    // ���ü�����
		SStringW _objId;         // ����ID
		SStringW _objName;       // �������� 
		SStringW _userData;      // �û�����
		SStringW _cursorName;    // ������� 
		IRichEditObjHost* _pObjHost;      // ����richedit 
		CHARRANGE _contentChr;    // ��richedit������ַ��±�,�����Ϣ����Ҫ
		CRect _marginRect;    // �������߾� 
		CRect _objRect;       // ��richedit�����λ�� 
		BOOL _isDirty;       // λ����Ϣ�ı��� 
		AlignType _alignType;     // ���뷽ʽ,��ָ����Ķ��뷽ʽ
	};
#pragma endregion

#pragma region RichEditText
	// RichEdit�ı����������塢��ɫ������
	class RichEditText : public RichEditObj
	{
		DEF_SOBJECT(RichEditText, L"text")
	public:
		RichEditText();
		~RichEditText() {}
		
		static RichEditObj* CreateObject() { return new RichEditText(); }

		static SStringW MakeFormatedText(const SStringW& text,
			int fontSize = 10,
			const SStringW& font_face = L"微软雅黑");

		virtual BOOL InitFromXml(IXmlNode* pNode);
		virtual void DrawObject(IRenderTarget*);
		virtual void UpdatePosition();
		virtual CRect GetRect();
		virtual BOOL PointInObject(POINT pt);
		virtual BOOL GetHitTestable();
		virtual BOOL NeedToProcessMessage();
		virtual LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual BOOL InsertIntoHost(IRichEditObjHost* pHost);
		int GetLineCount() { return _lineCount; }
		SStringW GetType() { return GetClassName(); }
		SStringW GetText() { return _text; }
		SStringW GetLinkData() { return _linkData; }
		SStringW GetTextType() { return _textType; }
		void SetText(LPCWSTR pszText);
		void SetLink(BOOL isLink);
		void SetTextStyle(BOOL underline, BOOL bold, BOOL italic, COLORREF color);
		void SetTextOffset(LONG loffset);

	protected:
		void FixText();
		SOUI_ATTRS_BEGIN()
			ATTR_STRINGW(L"font-face", _font, FALSE)
			ATTR_COLOR(L"color", _textColor, FALSE)
			ATTR_INT(L"underline", _isUnderline, FALSE)
			ATTR_INT(L"bold", _isBold, FALSE)
			ATTR_INT(L"italic", _isItalic, FALSE)
			ATTR_INT(L"font-size", _fontSize, FALSE)
			ATTR_INT(L"link", _isLink, FALSE)
			ATTR_STRINGW(L"link-data", _linkData, FALSE)
			ATTR_STRINGW(L"text-type", _textType, FALSE)
			ATTR_STRINGW(L"font_offset", _fontOffset, FALSE)
			SOUI_ATTRS_END()

	private:
		typedef std::vector<CRect> RectVec;
		RectVec     _objRects;
		DWORD       _status;
		COLORREF    _textColor;
		SStringW    _font;
		SStringW    _text;
		SStringW    _linkData;
		SStringW    _textType;
		int         _lineCount;
		BOOL        _isUnderline;
		BOOL        _isBold;
		BOOL        _isItalic;
		BOOL        _isLink;
		int         _fontSize;
		SStringW	_fontOffset;
	};
#pragma endregion

#pragma region RichEditBkElement
	// ����Ԫ��
	class RichEditBkElement : public RichEditObj
	{
		DEF_SOBJECT(RichEditBkElement, L"bkele")
	public:
		RichEditBkElement();
		~RichEditBkElement();
		
		static RichEditObj* CreateObject() { return new RichEditBkElement(); }

		void SetVisible(BOOL visible) { _bVisible = visible; }
		void SetInteractive(BOOL b) { _isInteractive = b; }
		SStringW GetType() { return GetClassName(); }
		BOOL GetHitTestable() { return _bVisible && _hittestable; }
		BOOL NeedToProcessMessage() { return _bVisible && _isInteractive; }
		void CalcPosition(POS_INFO* pItemsPos, int nPosCount);
		CRect GetRect();
		void SetText(const SStringW& text);
		void SetSkin(ISkinObj* pSkin, BOOL bAutoFree = TRUE);
		void SetTextColor(COLORREF cr) { _textColor = cr; }

	protected:
		LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		void BeforePaint(IRenderTarget* pRT, SPainter& painter);
		void AfterPaint(IRenderTarget* pRT, SPainter& painter);
		void DrawObject(IRenderTarget*);
		int PositionItem2Value(const POS_INFO& pos, int nMin, int nMax, BOOL bX);
		BOOL ParsePosition34(POS_INFO* pPosItem, const SStringW& strPos3, const SStringW& strPos4);
		BOOL ParsePosition12(POS_INFO* pPosItem, const SStringW& strPos1, const SStringW& strPos2);
		BOOL StrPos2ItemPos(const SStringW& strPos, POS_INFO& pos);
		HRESULT OnAttrSkin(const SStringW& strValue, BOOL bLoading);
		HRESULT OnAttrPos(const SStringW& strValue, BOOL bLoading);
		HRESULT OnAttrPosLeft(const SStringW& strValue, BOOL bLoading);
		HRESULT OnAttrPosCenter(const SStringW& strValue, BOOL bLoading);
		HRESULT OnAttrPosRight(const SStringW& strValue, BOOL bLoading);
		HRESULT OnInternalAttrPos(POS_INFO* pPosItem, int& nPosCount, const SStringW& strValue, BOOL bLoading);

		SOUI_ATTRS_BEGIN()
			ATTR_INT(L"visible", _bVisible, FALSE)
			ATTR_INT(L"interactive", _isInteractive, FALSE)
			ATTR_INT(L"hittestable", _hittestable, FALSE)
			ATTR_CUSTOM(L"pos", OnAttrPos)     
			ATTR_CUSTOM(L"center-pos", OnAttrPosCenter)
			ATTR_CUSTOM(L"left-pos", OnAttrPosLeft)
			ATTR_CUSTOM(L"right-pos", OnAttrPosRight)
			ATTR_CUSTOM(L"skin", OnAttrSkin)  
			ATTR_SKIN(L"left-skin", _pLeftSkin, FALSE)
			ATTR_SKIN(L"center-skin", _pCenterSkin, FALSE)
			ATTR_SKIN(L"right-skin", _pRightSkin, FALSE)
			ATTR_STRINGW(L"font", _font, FALSE)
			ATTR_COLOR(L"text-color", _textColor, FALSE)
			ATTR_STRINGW(L"text", _text, FALSE)
			ATTR_HEX(L"text-format", _textFormat, FALSE)
			ATTR_STRINGW(L"data", _data, FALSE)
			SOUI_ATTRS_END()

	protected:
		int _textFormat;
		SStringW _font;
		SStringW _text;
		COLORREF _textColor;
		BOOL _bVisible;          // �Ƿ���ʾ
		DWORD _status;            // ������״̬�����������ж��Ƿ���������
		BOOL _isInteractive;     // �Ƿ���Ҫ������Ϣ������WM_MOUSEMOVE
		BOOL _hittestable;       // �ܷ�hittest

		int _defPosCount;       // ���������ʱ��������� */
		POS_INFO _defPosItems[4];    // ��pos���Զ����ֵ, _defPosCount >0 ʱ��Ч*/
		int _centerPosCount;    // ������ж���ʱ��������� */
		POS_INFO _centerPosItems[4]; // ��pos���Զ����ֵ, _centerPosCount >0 ʱ��Ч*/
		int _rightPosCount;     // �����Ҷ���ʱ��������� */
		POS_INFO _rightPosItems[4];  // ��pos���Զ����ֵ, _rightPosCount >0 ʱ��Ч*/

		ISkinObj* _pLeftSkin;         // Ĭ��ͼƬ��һ����������skin
		ISkinObj* _pCenterSkin;       // ���ж����skin
		ISkinObj* _pRightSkin;        // �Ҷ����skin

		SStringW _data;              // �û�����
	};
#pragma endregion

#pragma region RichEditPara
	class RichEditPara : public RichEditObj
	{
		DEF_SOBJECT(RichEditObj, L"para")
	public:
		RichEditPara();
		~RichEditPara() {}
		
		static RichEditObj* CreateObject() { return new RichEditPara(); }

		SStringW GetType() { return GetClassName(); }
		BOOL GetHitTestable() { return FALSE; }
		CRect GetRect();
		void UpdatePosition();
		BOOL InsertIntoHost(IRichEditObjHost* pHost);
		void SetAlign(AlignType align);
		BOOL IsWrapped();
		void OffsetCharRange(int nOffset, BOOL bUpdate = FALSE);
		void SetCharRange(const CHARRANGE& chr);
	protected:
		BOOL GetAutoWrapped();
		BOOL CalculateRect();
		BOOL GetLineRect(int nLineNo, CRect& rcLine);

		SOUI_ATTRS_BEGIN()
			ATTR_INT(L"break", _breakAtTheEnd, FALSE)
			ATTR_INT(L"simulate-align", _simulateAlign, FALSE)
			ATTR_INT(L"disable-layout", _disableLayout, FALSE)
			SOUI_ATTRS_END()

		BOOL _autoWrapped; 
		BOOL _needUpdateLayout;
		BOOL _simulateAlign;
		int _breakAtTheEnd; 
		int _lineCount; 
		BOOL _initialized;
		BOOL _disableLayout;
	};
#pragma endregion

#pragma region RichEditContent
	class RichEditContent : public RichEditObj
	{
#define THRESHOLD_FOR_AUTOLAYOUT 2400 
		DEF_SOBJECT(RichEditObj, L"RichEditContent")

	public:
		RichEditContent() :_autoLayout(FALSE) {}
		~RichEditContent() {}
		
		static RichEditObj* CreateObject() { return new RichEditContent(); }

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
			ATTR_INT(L"auto-layout", _autoLayout, TRUE) //是否自动布局,该设置会覆盖align属性
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
#pragma endregion

}



