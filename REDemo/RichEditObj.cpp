#include "stdafx.h"
#include "RichEditObj.h"

namespace SOUI
{
#define _tomClientCoord     256  // Ĭ�ϻ�ȡ��������Ļ���꣬ Use client coordinates instead of screen coordinates.
#define _tomAllowOffClient  512  // Allow points outside of the client area.

	const int  LEFT = 0;
	const int  TOP = 1;
	const int  RIGHT = 2;
	const int  BOTTOM = 3;

	// local helpers
	float px2pt(int px)
	{
		static int dpi = 0;
		if (!dpi)
		{
			HDC hDC = GetDC(NULL);
			dpi = GetDeviceCaps(hDC, LOGPIXELSX);
			ReleaseDC(NULL, hDC);
		}

		return px * 72 / (float)dpi;
	}

#pragma region RichEditObj
	RichEditObj::RichEditObj()
		:_pParent(NULL)
		, _pFirstChild(NULL)
		, _pLastChild(NULL)
		, _pNextSibling(NULL)
		, _pPrevSibling(NULL)
		, _childrenCount(0)
		, _pObjHost(NULL)
		, _alignType(ALIGN_LEFT)
		, _references(1)
		, _cursorName(L"arrow")
	{
		_contentChr.cpMin = -1;
		_contentChr.cpMax = -1;
	}

	RichEditObj::~RichEditObj()
	{
		OnDestroy();
	}

	LONG RichEditObj::AddRef(void)
	{
		return ++_references;
	}

	LONG RichEditObj::Release(void)
	{
		if (0 == --_references)
		{
			delete this;
			return 0;
		}
		return _references;
	}

	void RichEditObj::DestroyObject()
	{
		if (!GetParent())
			OnDestroy();
		else
			GetParent()->DestroyChild(this);
	}

	BOOL RichEditObj::DestroyChild(RichEditObj* pChild)
	{
		if (this != pChild->GetParent())
			return FALSE;

		RemoveChild(pChild);
		pChild->Release();

		return TRUE;
	}

	void RichEditObj::OnDestroy()
	{
		//destroy children objects
		RichEditObj* pChild = _pFirstChild;
		while (pChild)
		{
			RichEditObj* pNextChild = pChild->GetNext();
			pChild->Release();

			pChild = pNextChild;
		}

		_pFirstChild = _pLastChild = NULL;
		_childrenCount = 0;
	}

	UINT RichEditObj::GetChildrenCount()
	{
		return _childrenCount;
	}

	void RichEditObj::InsertChild(RichEditObj* pNewChild, RichEditObj* pInsertAfter/*=REOBJ_LAST*/)
	{
		if (pNewChild->_pParent == this)
			return;

		pNewChild->_pParent = this;
		pNewChild->_pPrevSibling = pNewChild->_pNextSibling = NULL;

		if (pInsertAfter == _pLastChild) pInsertAfter = REOBJ_LAST;

		if (pInsertAfter == REOBJ_LAST)
		{
			//insert obj at head
			pNewChild->_pPrevSibling = _pLastChild;
			if (_pLastChild) _pLastChild->_pNextSibling = pNewChild;
			else _pFirstChild = pNewChild;
			_pLastChild = pNewChild;
		}
		else if (pInsertAfter == REOBJ_FIRST)
		{
			//insert obj at tail
			pNewChild->_pNextSibling = _pFirstChild;
			if (_pFirstChild) _pFirstChild->_pPrevSibling = pNewChild;
			else _pLastChild = pNewChild;
			_pFirstChild = pNewChild;
		}
		else
		{
			//insert obj at middle
			SASSERT(pInsertAfter->_pParent == this);
			SASSERT(_pFirstChild && _pLastChild);
			RichEditObj* pNext = pInsertAfter->_pNextSibling;
			SASSERT(pNext);
			pInsertAfter->_pNextSibling = pNewChild;
			pNewChild->_pPrevSibling = pInsertAfter;
			pNewChild->_pNextSibling = pNext;
			pNext->_pPrevSibling = pNewChild;
		}
		_childrenCount++;
	}

	BOOL RichEditObj::RemoveChild(RichEditObj* pChild)
	{
		if (this != pChild->GetParent())
			return FALSE;

		RichEditObj* pPrevSib = pChild->_pPrevSibling;
		RichEditObj* pNextSib = pChild->_pNextSibling;

		if (pPrevSib)
			pPrevSib->_pNextSibling = pNextSib;
		else
			_pFirstChild = pNextSib;

		if (pNextSib)
			pNextSib->_pPrevSibling = pPrevSib;
		else
			_pLastChild = pPrevSib;

		pChild->_pParent = NULL;
		pChild->_pNextSibling = NULL;
		pChild->_pPrevSibling = NULL;
		_childrenCount--;

		return TRUE;
	}

	RichEditObj* RichEditObj::GetById(LPCWSTR lpszId)
	{
		if (lpszId == _objId)
			return this;

		for (RichEditObj* p = _pFirstChild; p != NULL; p = p->GetNext())
		{
			RichEditObj* pObjMatched = p->GetById(lpszId);
			if (pObjMatched)
				return pObjMatched;
		}

		return NULL;
	}

	RichEditObj* RichEditObj::GetByName(LPCWSTR lpszName)
	{
		if (lpszName == _objName)
			return this;

		for (RichEditObj* p = _pFirstChild; p != NULL; p = p->GetNext())
		{
			RichEditObj* pObjMatched = p->GetByName(lpszName);
			if (pObjMatched)
				return pObjMatched;
		}

		return NULL;
	}

	RichEditObj* RichEditObj::FindChildByName(LPCWSTR pszName, int nDeep)
	{
		if (!pszName || nDeep == 0) return NULL;

		RichEditObj* pChild = GetFirstChild();
		while (pChild)
		{
			if (pChild->_objName == pszName)
				return pChild;
			pChild = pChild->GetNext();
		}

		if (nDeep > 0) nDeep--;
		if (nDeep == 0) return NULL;

		pChild = GetFirstChild();
		while (pChild)
		{
			RichEditObj* pChildFind = pChild->FindChildByName(pszName, nDeep);
			if (pChildFind) return pChildFind;
			pChild = pChild->GetNext();
		}

		return NULL;
	}

	void RichEditObj::UpdatePosition()
	{
		for (RichEditObj* p = _pFirstChild; p != NULL; p = p->GetNext())
			p->UpdatePosition();
	}

	void RichEditObj::SetAlign(AlignType type)
	{
		_alignType = type;
		for (RichEditObj* p = _pFirstChild; p != NULL; p = p->GetNext())
			p->SetAlign(type);
	}

	void RichEditObj::OffsetCharRange(int nOffset, BOOL bUpdate/*=FALSE*/)
	{
		_contentChr.cpMin += nOffset;
		_contentChr.cpMax += nOffset;
		for (RichEditObj* p = _pFirstChild; p != NULL; p = p->GetNext())
			p->OffsetCharRange(nOffset, bUpdate);
	}

	void RichEditObj::ExpandCharRange(int startCp, int nOffset, BOOL bUpdate/*=FALSE*/)
	{
		if (_contentChr.cpMin > startCp)
			_contentChr.cpMin += nOffset;

		_contentChr.cpMax += nOffset;

		for (RichEditObj* p = _pFirstChild; p != NULL; p = p->GetNext())
			p->ExpandCharRange(startCp, nOffset, bUpdate);
	}

	void RichEditObj::SetDirty(BOOL bDirty)
	{
		_isDirty = bDirty;
		for (RichEditObj* p = _pFirstChild; p != NULL; p = p->GetNext())
			p->SetDirty(bDirty);
	}

	void RichEditObj::AdjustMessageParam(UINT msg, WPARAM& wParam, LPARAM& lParam)
	{
		switch (msg)
		{
		case WM_MOUSEMOVE:
		case WM_MOUSELEAVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_SETCURSOR:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			CRect rcObj = GetRect();
			POINT ptOrgin = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			int x = ptOrgin.x - rcObj.left;
			int y = ptOrgin.y - rcObj.top;
			lParam = MAKELPARAM(x, y);
			break;
		}
	}

	LRESULT RichEditObj::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (msg == WM_SETCURSOR && !_cursorName.IsEmpty())
		{
			HCURSOR  hCursor = GETRESPROVIDER->LoadCursor(_cursorName);
			::SetCursor(hCursor);
		}

		for (RichEditObj* p = _pFirstChild; p != NULL && !bHandled; p = p->GetNext())
		{
			p->ProcessMessage(msg, wParam, lParam, bHandled);
			if (bHandled)
				return 0;
		}

		return 0;
	}

	BOOL RichEditObj::OnUpdateToolTip(CPoint pt, SwndToolTipInfo& tipInfo)
	{
		for (RichEditObj* p = _pFirstChild; p != NULL; p = p->GetNext())
		{
			if (p->OnUpdateToolTip(pt, tipInfo))
				return TRUE;
		}

		return FALSE;
	}

	void RichEditObj::DrawObject(IRenderTarget* pRT)
	{
		for (RichEditObj* p = _pFirstChild; p != NULL; p = p->GetNext())
			p->DrawObject(pRT);
	}

	BOOL RichEditObj::InsertIntoHost(IRichEditObjHost* pHost)
	{
		SASSERT(pHost);
		_pObjHost = pHost;

		CHARRANGE chr;
		_pObjHost->SendMessage(EM_EXGETSEL, NULL, (LPARAM)&chr);
		_contentChr.cpMin = chr.cpMax;
		int nContentLen = pHost->GetCharCount();

		RichEditObj* p = _pFirstChild;
		while (p)
		{
			p->InsertIntoHost(pHost);
			p = p->GetNext();
		}

		_contentChr.cpMax = _contentChr.cpMin + pHost->GetCharCount() - nContentLen;
		return TRUE;
	}

	BOOL RichEditObj::InitFromXml(IXmlNode* pNode)
	{
		if (pNode)
		{
			SObject::InitFromXml(pNode);
			CreateChildren(pNode);
		}

		return TRUE;
	}

	BOOL RichEditObj::CreateChildren(IXmlNode* pNode)
	{
		if (!pNode)
			return FALSE;

		SStringW sstrTmp1;
		pNode->ToString(&sstrTmp1);

		SXmlNode node(pNode);
		for (SXmlNode xmlChild = node.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
		{
			if (xmlChild.type() != pugi::node_element) continue;
			RichEditObj* pChild = RichEditObjFactory::GetInstance().CreateObjectByName(xmlChild.name());
			if (pChild)
			{
				InsertChild(pChild);
				SStringW sstrTmp;
				xmlChild.ToString(&sstrTmp);
				pChild->InitFromXml(&xmlChild);
			}
		}
		return TRUE;
	}
#pragma endregion

#pragma region RichEditText
	RichEditText::RichEditText() : _textColor(RGB(0, 0, 0))
		, _lineCount(1)
		, _isUnderline(FALSE)
		, _isBold(FALSE)
		, _isItalic(FALSE)
		, _fontSize(-1)
		, _isLink(FALSE)
		, _status(0)
	{
	}


	BOOL RichEditText::InitFromXml(IXmlNode* pNode)
	{
		_text = pNode->Text();

		return __super::InitFromXml(pNode);
	}

	void RichEditText::FixText()
	{
		int length = _pObjHost->GetRemainingLength();

		if (_text.GetLength() >= length)
		{
			_text = _text.Left(length);
		}

		_lineCount = 1;
		for (int nlf = _text.Find(_T("\n")); nlf >= 0; ++_lineCount)
		{
			nlf = _text.Find(_T("\n"), nlf + 1);
		}
	}

	void RichEditText::DrawObject(IRenderTarget*)
	{
		if (_isDirty)
		{
			UpdatePosition();
		}
		_isDirty = FALSE;
	}

	void RichEditText::UpdatePosition()
	{
		if (!_isLink)
			return;

		_objRects.clear();
		int lineStart = 0;
		int lineEnd = 0;
		_pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMin, (LRESULT*)&lineStart);
		_pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMax, (LRESULT*)&lineEnd);

		LONG chrIndex = _contentChr.cpMin;
		for (int lineIndex = lineStart; lineIndex <= lineEnd; ++lineIndex)
		{
			int startCp = 0;
			int lineLength = 0;
			_pObjHost->SendMessage(EM_LINEINDEX, lineIndex, 0, (LRESULT*)&startCp);
			_pObjHost->SendMessage(EM_LINELENGTH, startCp, 0, (LRESULT*)&lineLength);

			CHARRANGE chr = { chrIndex, chrIndex };
			while (chrIndex < _contentChr.cpMax - 1 && chrIndex < startCp + lineLength)
			{
				chrIndex += 1;
				chr.cpMax += 1;
			}

			// ���㵱ǰ�е�rect
			CRect rect;
			SComPtr<ITextRange>  spRangeLine;
			ITextDocument* pdoc = _pObjHost->GetTextDoc();
			pdoc->Range(chr.cpMin, chr.cpMax, &spRangeLine);

			if (!spRangeLine)
				continue;

			long lTypeTopLeft = _tomAllowOffClient | _tomClientCoord | tomStart | TA_TOP | TA_LEFT;
			long lTypeRightBottom = _tomAllowOffClient | _tomClientCoord | tomEnd | TA_BOTTOM | TA_RIGHT;
			POINT   ptEnd, ptStart;
			spRangeLine->GetPoint(lTypeTopLeft, &ptStart.x, &ptStart.y);
			spRangeLine->GetPoint(lTypeRightBottom, &ptEnd.x, &ptEnd.y);
			rect.SetRect(ptStart, ptEnd);
			_objRects.push_back(rect);
		}
	}

	BOOL RichEditText::PointInObject(POINT pt)
	{
		RectVec::iterator it = _objRects.begin();
		for (; it != _objRects.end(); ++it)
		{
			if (it->PtInRect(pt))
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	CRect RichEditText::GetRect()
	{
		_objRect.SetRectEmpty();

		RectVec::iterator it = _objRects.begin();
		for (; it != _objRects.end(); ++it)
		{
			_objRect.UnionRect(_objRect, *it);
		}

		return _objRect;
	}

	BOOL RichEditText::GetHitTestable()
	{
		return _isLink;
	}

	BOOL RichEditText::NeedToProcessMessage()
	{
		return _isLink;
	}

	LRESULT RichEditText::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		if (!PointInObject(pt))
			return 0;

		//bHandled = TRUE;
		HCURSOR hCursor = NULL;
		switch (msg)
		{
		case WM_MOUSEMOVE:
			hCursor = GETRESPROVIDER->LoadCursor(L"hand");
			::SetCursor(hCursor);
			break;

		case WM_LBUTTONDOWN:
			SetCapture(_pObjHost->GetHostContainer()->GetHostHwnd());
			_status |= WndState_PushDown;
			break;

		case WM_LBUTTONUP:
			::ReleaseCapture();

			if (_status & WndState_PushDown)
			{
				_status &= ~WndState_PushDown;
				_pObjHost->NotifyRichObjEvent(this, CLICK_LINK, 0, 0);
			}
			break;

		default:
			bHandled = FALSE;
			break;
		}
		return 0;
	}

	void RichEditText::SetText(LPCWSTR pszText)
	{
		if (!pszText || _text == pszText)
			return;

		if (_contentChr.cpMin < _contentChr.cpMax)
		{
			_text = pszText;

			int length = _pObjHost->GetCharCount();
			_pObjHost->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&_contentChr);
			_pObjHost->SendMessage(EM_REPLACESEL, TRUE, (LPARAM)(LPCWSTR)_text);
			length += _pObjHost->GetCharCount() - length;

			//CHARRANGE chr = { length, length };
			//_pObjHost->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&chr);
		}
	}

	void RichEditText::SetLink(BOOL isLink)
	{
		_isLink = isLink;
	}

	void RichEditText::SetTextOffset(LONG loffset)
	{
		CHARFORMATW cf = { 0 };
		cf.cbSize = sizeof(CHARFORMATW);
		cf.yOffset = loffset;

		_pObjHost->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&_contentChr);
		_pObjHost->SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

		CHARRANGE chr = { _contentChr.cpMax, _contentChr.cpMax };
		_pObjHost->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&chr);
	}

	void RichEditText::SetTextStyle(BOOL underline, BOOL bold, BOOL italic, COLORREF color)
	{
		_textColor = color & 0x00ffffff;
		_isUnderline = underline;
		_isBold = bold;
		_isItalic = italic;

		// ����������ʽ
		CHARFORMATW cf = { 0 };
		cf.cbSize = sizeof(CHARFORMATW);

		cf.dwMask = CFM_COLOR;
		cf.crTextColor = _textColor & 0x00ffffff;

		if (_isUnderline)
		{
			cf.dwMask |= CFM_UNDERLINE;
			cf.dwEffects |= CFE_UNDERLINE;
		}

		if (_isBold)
		{
			cf.dwMask |= CFM_BOLD;
			cf.dwEffects |= CFE_BOLD;
		}

		if (_isItalic)
		{
			cf.dwMask |= CFM_ITALIC;
			cf.dwEffects |= CFE_ITALIC;
		}

		_pObjHost->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&_contentChr);
		_pObjHost->SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

		CHARRANGE chr = { _contentChr.cpMax, _contentChr.cpMax };
		_pObjHost->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&chr);
	}

	BOOL RichEditText::InsertIntoHost(IRichEditObjHost* pHost)
	{
		SASSERT(pHost);
		_pObjHost = pHost;
		_pObjHost->SendMessage(EM_EXGETSEL, NULL, (LPARAM)&_contentChr);

		FixText();

		int nLength = _pObjHost->GetCharCount();
		_pObjHost->SendMessage(EM_REPLACESEL, TRUE, (LPARAM)(LPCWSTR)_text);
		_contentChr.cpMin = _contentChr.cpMax;
		_contentChr.cpMax += _pObjHost->GetCharCount() - nLength;
		_pObjHost->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&_contentChr);

		// ����������ʽ
		CHARFORMATW cf = { 0 };
		cf.cbSize = sizeof(CHARFORMATW);

		cf.dwMask = CFM_COLOR;
		cf.crTextColor = _textColor & 0x00ffffff;

		cf.dwMask |= CFM_UNDERLINE;
		if (_isUnderline)
		{
			cf.dwEffects |= CFE_UNDERLINE;
		}

		cf.dwMask |= CFM_BOLD;
		if (_isBold)
		{
			cf.dwEffects |= CFE_BOLD;
		}

		cf.dwMask |= CFM_ITALIC;
		if (_isItalic)
		{
			cf.dwEffects |= CFE_ITALIC;
		}

		if (!_font.IsEmpty())
		{
			cf.dwMask |= CFM_FACE;
			wcscpy_s(cf.szFaceName, LF_FACESIZE - 1, _font);
		}

		if (_fontSize > 0)
		{
			cf.dwMask |= CFM_SIZE;

			FLOAT px;
			FLOAT twips;

			RichEditUintConverter::PointToPixel((FLOAT)_fontSize, 96, px);
			RichEditUintConverter::PixelToTwips(px, twips);
			cf.yHeight = (LONG)twips;

			//cf.yHeight = _fontSize * 20;
		}

		_pObjHost->SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

		//if (_isLink && !_linkData.IsEmpty())
		//{
		//    CComPtr<ITextRange2> range2;
		//    ITextDocument2* pDoc = _pObjHost->GetTextDoc();
		//    pDoc->Range2(_contentChr.cpMin, _contentChr.cpMax, &range2);

		//    SStringW url = _T("\"");
		//    url += _linkData;
		//    url += _T("\"");
		//    BSTR bstr = ::SysAllocString((LPCWSTR)url);

		//    HRESULT hr = range2->SetURL(bstr);
		//    SASSERT(SUCCEEDED(hr));
		//    ::SysFreeString(bstr);
		//}

		// �ѹ���Ƶ����
		CHARRANGE chr = { _contentChr.cpMax, _contentChr.cpMax };
		_pObjHost->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&chr);

		return TRUE;
	}

	//
	// �����ָ�ʽ����RichEditText�ĸ�ʽ
	//
	// @param text: ��Ҫ��ʽ�����ı�����
	// @param fontSize: �����С����λ��pt
	//
	SStringW RichEditText::MakeFormatedText(const SStringW& text,
		int fontSize/*=10*/,
		const SStringW& font_face/* = L"微软雅黑*/)
	{
		SStringW formattedText;

		formattedText.Format(_T("<text font-size=\"%d\" font-face=\"%s\"><![CDATA["), fontSize, font_face);
		formattedText += text;

		SStringW splitStr;
		splitStr.Format(_T("]]]]></text><text font-size=\"%d\"><![CDATA[>"), fontSize);
		formattedText.Replace(_T("]]>"), splitStr);
		formattedText += _T("]]></text>");

		return formattedText;
	}
#pragma endregion

#pragma region RichEditBkElement
	RichEditBkElement::RichEditBkElement()
		: _status(WndState_Normal)
		, _isInteractive(FALSE)
		, _hittestable(TRUE)
		, _pLeftSkin(NULL)
		, _pCenterSkin(NULL)
		, _pRightSkin(NULL)
		, _defPosCount(0)
		, _centerPosCount(0)
		, _rightPosCount(0)
		, _font(L"size:12")
		, _textFormat(0)
		, _bVisible(TRUE)
		, _textColor(CR_INVALID)
	{
		memset(&_defPosItems, 0, sizeof(_defPosItems));
		memset(&_centerPosItems, 0, sizeof(_centerPosItems));
		memset(&_rightPosItems, 0, sizeof(_rightPosItems));
	}

	RichEditBkElement::~RichEditBkElement()
	{
	}

	LRESULT RichEditBkElement::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		if (!GetRect().PtInRect(pt))
			return 0;

		bHandled = TRUE;
		HCURSOR hCursor = NULL;

		switch (msg)
		{
		case WM_SETCURSOR:
			hCursor = GETRESPROVIDER->LoadCursor(_cursorName);
			::SetCursor(hCursor);
			break;

		case WM_LBUTTONDOWN:
			SetCapture(_pObjHost->GetHostContainer()->GetHostHwnd());
			_status |= WndState_PushDown;
			break;

		case WM_LBUTTONUP:
			::ReleaseCapture();

			if (_status & WndState_PushDown)
			{
				_pObjHost->NotifyRichObjEvent(this, CLICK_BK_ELE, 0, 0);
			}
			_status &= ~WndState_PushDown;
			break;

		default:
			bHandled = FALSE;
			break;
		}

		return 0;
	}

	void RichEditBkElement::BeforePaint(IRenderTarget* pRT, SPainter& painter)
	{
		/*IFontPtr pFont = SFontPool::getSingleton().GetFont(_font, GetScale());*/
		IFontPtr pFont = SFontPool::GetFont(_font, GetScale());
		if (pFont)
		{
			pRT->SelectObject(pFont, (IRenderObj**)&painter.oldFont);
		}

		if (_textColor != CR_INVALID)
		{
			painter.oldTextColor = pRT->SetTextColor(_textColor);
		}
	}

	void RichEditBkElement::AfterPaint(IRenderTarget* pRT, SPainter& painter)
	{
		if (painter.oldFont) pRT->SelectObject(painter.oldFont);
		if (painter.oldTextColor != CR_INVALID) pRT->SetTextColor(painter.oldTextColor);
	}

	void RichEditBkElement::DrawObject(IRenderTarget* pRT)
	{
		if (!_bVisible)
			return;

		POS_INFO pos[4];
		INT posCount = 0;
		ISkinObj* pSkin = NULL;

		if (_alignType == ALIGN_RIGHT)
		{
			memcpy(&pos, &_rightPosItems, sizeof(_rightPosItems));
			posCount = _rightPosCount;
			pSkin = _pRightSkin;
		}
		else if (_alignType == ALIGN_CENTER)
		{
			memcpy(&pos, &_centerPosItems, sizeof(_centerPosItems));
			posCount = _centerPosCount;
			pSkin = _pCenterSkin;
		}
		else
		{
			memcpy(&pos, &_defPosItems, sizeof(_defPosItems));
			posCount = _defPosCount;
			pSkin = _pLeftSkin;
		}

		if (_isDirty)
		{
			// ����������ǰһ��/��һ���ֵܽڵ��ȸ���λ����Ϣ���
			CalcPosition(pos, posCount);
		}

		if (pSkin)
		{
			CRect rcHost = _pObjHost->GetAdjustedRect();
			CRect rcTemp = rcHost;

			if (rcHost.IntersectRect(_objRect, rcTemp))
			{
				pRT->PushClipRect(rcHost);
				//pSkin->Draw(pRT, _objRect, 0);
				pRT->PopClip();
			}
		}

		//pRT->FillSolidRect(_objRect, RGBA(128, 128, 128, 255));
		if (_text.GetLength() > 0)
		{
			SPainter painter;
			BeforePaint(pRT, painter);
			pRT->DrawText(_text, _text.GetLength(), _objRect, _textFormat);
			AfterPaint(pRT, painter);
		}
	}

	BOOL RichEditBkElement::StrPos2ItemPos(const SStringW& strPos, POS_INFO& pos)
	{
		if (strPos.IsEmpty()) return FALSE;

		LPCWSTR pszPos = strPos;
		switch (pszPos[0])
		{
			//case POSFLAG_REFCENTER: pos.pit=PIT_CENTER,pszPos++;break;
			//case POSFLAG_PERCENT: pos.pit=PIT_PERCENT,pszPos++;break;
		case POSFLAG_REFPREV_NEAR: pos.pit = PIT_PREV_NEAR, pszPos++; break;
		case POSFLAG_REFNEXT_NEAR: pos.pit = PIT_NEXT_NEAR, pszPos++; break;
		case POSFLAG_REFPREV_FAR: pos.pit = PIT_PREV_FAR, pszPos++; break;
		case POSFLAG_REFNEXT_FAR: pos.pit = PIT_NEXT_FAR, pszPos++; break;
		case POSFLAG_SIZE:pos.pit = PIT_SIZE, pszPos++; break;
		default: pos.pit = PIT_NORMAL; break;
		}

		pos.nRefID = -1;//not ref sibling using id
		if (pszPos[0] == L'-')
		{
			pos.bMinus = -1;
			pszPos++;
		}
		else
		{
			pos.bMinus = 1;
		}
		pos.nPos.fSize = (float)_wtof(pszPos);

		return TRUE;
	}

	BOOL RichEditBkElement::ParsePosition34(POS_INFO* pPosItem, const SStringW& strPos3, const SStringW& strPos4)
	{
		if (strPos3.IsEmpty() || strPos4.IsEmpty()) return FALSE;
		POS_INFO pos3, pos4;
		if (!StrPos2ItemPos(strPos3, pos3) || !StrPos2ItemPos(strPos4, pos4)) return FALSE;

		pPosItem[RIGHT] = pos3;
		pPosItem[BOTTOM] = pos4;
		return TRUE;
	}

	BOOL RichEditBkElement::ParsePosition12(POS_INFO* pPosItem, const SStringW& strPos1, const SStringW& strPos2)
	{
		if (strPos1.IsEmpty() || strPos2.IsEmpty())
			return FALSE;
		POS_INFO pos1, pos2;
		if (!StrPos2ItemPos(strPos1, pos1) || !StrPos2ItemPos(strPos2, pos2))
			return FALSE;
		if (pos1.pit == PIT_SIZE || pos2.pit == PIT_SIZE)//ǰ��2�����Բ�����size����
			return FALSE;
		pPosItem[LEFT] = pos1;
		pPosItem[TOP] = pos2;
		return TRUE;
	}

	HRESULT RichEditBkElement::OnInternalAttrPos(POS_INFO* pPosItem, int& nPosCount, const SStringW& strValue, BOOL bLoading)
	{
		SStringWList strLst;
		SplitString(strValue, L',', strLst);
		if (strLst.GetCount() != 2 && strLst.GetCount() != 4)
		{
			SASSERT_FMTW(L"Parse pos attribute failed, strPos=%s", strValue);
			return FALSE;
		}

		//����pos�����еĿո���ݡ�
		for (size_t i = 0; i < strLst.GetCount(); i++)
		{
			strLst.GetAt(i).TrimBlank();
		}
		BOOL bRet = TRUE;
		BOOL bRet2 = TRUE;

		bRet = ParsePosition12(pPosItem, strLst[0], strLst[1]);
		if (strLst.GetCount() == 4)
		{
			bRet2 = ParsePosition34(pPosItem, strLst[2], strLst[3]);
		}

		if (bRet)
		{
			nPosCount = 2;
		}
		if (bRet2)
		{
			nPosCount = 4;
		}
		return bRet && bRet2;
	}

	int RichEditBkElement::PositionItem2Value(const POS_INFO& pos, int nMin, int nMax, BOOL bX)
	{
		int nRet = 0;

		switch (pos.pit)
		{
		case PIT_NORMAL:
			if (pos.bMinus == -1)
				nRet = nMax - (int)pos.nPos.fSize;
			else
				nRet = nMin + (int)pos.nPos.fSize;
			break;

		case PIT_PREV_NEAR: //��[�������ǰһ�ֵܴ��ڡ�����Xʱ���ο�ǰһ�ֵܴ��ڵ�right������Yʱ�ο�ǰһ�ֵܴ��ڵ�bottom
		case PIT_PREV_FAR:  //��{�������ǰһ�ֵܴ��ڡ�����Xʱ���ο�ǰһ�ֵܴ��ڵ�left������Yʱ�ο�ǰһ�ֵܴ��ڵ�top
		{
			CRect rcRef;
			RichEditObj* pRefObj = GetPrev();
			if (pRefObj)
			{
				rcRef = pRefObj->GetRect();
			}
			else
			{
				rcRef = _pObjHost->GetAdjustedRect();
				rcRef.right = rcRef.left;
				rcRef.bottom = rcRef.top;
			}
			if (bX)
			{
				LONG refPos = (pos.pit == PIT_PREV_NEAR) ? rcRef.right : rcRef.left;
				nRet = refPos + (int)pos.nPos.fSize * pos.bMinus;
			}
			else
			{
				LONG refPos = (pos.pit == PIT_PREV_NEAR) ? rcRef.bottom : rcRef.top;
				nRet = refPos + (int)pos.nPos.fSize * pos.bMinus;
			}
		}
		break;

		case PIT_NEXT_NEAR: //��]������ں�һ�ֵܴ��ڡ�����Xʱ���ο���һ�ֵܵ�left,����Yʱ�ο���һ�ֵܵ�top
		case PIT_NEXT_FAR:  //��}������ں�һ�ֵܴ��ڡ�����Xʱ���ο���һ�ֵܵ�right,����Yʱ�ο���һ�ֵܵ�bottom
		{
			CRect rcRef;
			RichEditObj* pRefObj = GetNext();
			if (pRefObj)
			{
				rcRef = pRefObj->GetRect();
			}
			else
			{
				rcRef = _pObjHost->GetAdjustedRect();
				rcRef.left = rcRef.right;
				rcRef.top = rcRef.bottom;
			}

			if (bX)
			{
				LONG refPos = (pos.pit == PIT_NEXT_NEAR) ? rcRef.left : rcRef.right;
				nRet = refPos + (int)pos.nPos.fSize * pos.bMinus;
			}
			else
			{
				LONG refPos = (pos.pit == PIT_NEXT_NEAR) ? rcRef.top : rcRef.bottom;
				nRet = refPos + (int)pos.nPos.fSize * pos.bMinus;
			}
		}
		break;
		}

		return nRet;
	}

	CRect RichEditBkElement::GetRect()
	{
		POS_INFO pos[4];
		int posCount = 0;

		if (_alignType == ALIGN_RIGHT)
		{
			memcpy(&pos, &_rightPosItems, sizeof(_rightPosItems));
			posCount = _rightPosCount;
		}
		else if (_alignType == ALIGN_CENTER)
		{
			memcpy(&pos, &_centerPosItems, sizeof(_centerPosItems));
			posCount = _centerPosCount;
		}
		else
		{
			memcpy(&pos, &_defPosItems, sizeof(_defPosItems));
			posCount = _defPosCount;
		}

		if (_isDirty)
		{
			CalcPosition(pos, posCount);
			_isDirty = FALSE;
		}

		return _objRect;
	}

	void RichEditBkElement::SetText(const SStringW& text)
	{
		_text = text;
	}

	void RichEditBkElement::SetSkin(ISkinObj* pSkin, BOOL bAutoFree /* = TRUE */)
	{
		_pLeftSkin = pSkin;
		_pCenterSkin = pSkin;
		_pRightSkin = pSkin;
	}

	void RichEditBkElement::CalcPosition(POS_INFO* pItemsPos, int nPosCount)
	{
		if (!_bVisible || nPosCount == 0 || !_pObjHost)
		{
			return;
		}

		CRect rcHost = _pObjHost->GetAdjustedRect();

		// left
		_objRect.left = PositionItem2Value(pItemsPos[LEFT], rcHost.left, rcHost.right, TRUE);

		// top
		_objRect.top = PositionItem2Value(pItemsPos[TOP], rcHost.top, rcHost.bottom, FALSE);

		// right
		if (pItemsPos[RIGHT].pit == PIT_SIZE)
		{
			_objRect.right = _objRect.left + (LONG)pItemsPos[RIGHT].nPos.fSize;
		}
		else
		{
			_objRect.right = PositionItem2Value(pItemsPos[RIGHT], rcHost.left, rcHost.right, TRUE);
		}

		// bottom
		if (pItemsPos[BOTTOM].pit == PIT_SIZE)
		{
			_objRect.bottom = _objRect.top + (LONG)pItemsPos[BOTTOM].nPos.fSize;
		}
		else
		{
			_objRect.bottom = PositionItem2Value(pItemsPos[BOTTOM], rcHost.top, rcHost.bottom, FALSE);
		}
		_isDirty = FALSE;
	}

	HRESULT RichEditBkElement::OnAttrSkin(const SStringW& strValue, BOOL bLoading)
	{
		_pLeftSkin = GETSKIN(strValue, GetScale());
		_pRightSkin = _pLeftSkin;
		_pCenterSkin = _pLeftSkin;
		return FALSE;
	}

	HRESULT RichEditBkElement::OnAttrPos(const SStringW& strValue, BOOL bLoading)
	{
		OnInternalAttrPos(_defPosItems, _defPosCount, strValue, bLoading);
		OnInternalAttrPos(_centerPosItems, _rightPosCount, strValue, bLoading);
		OnInternalAttrPos(_rightPosItems, _rightPosCount, strValue, bLoading);
		return FALSE;
	}

	HRESULT RichEditBkElement::OnAttrPosLeft(const SStringW& strValue, BOOL bLoading)
	{
		return OnInternalAttrPos(_defPosItems, _defPosCount, strValue, bLoading);
	}

	HRESULT RichEditBkElement::OnAttrPosCenter(const SStringW& strValue, BOOL bLoading)
	{
		return OnInternalAttrPos(_centerPosItems, _rightPosCount, strValue, bLoading);
	}

	HRESULT RichEditBkElement::OnAttrPosRight(const SStringW& strValue, BOOL bLoading)
	{
		return OnInternalAttrPos(_rightPosItems, _rightPosCount, strValue, bLoading);
	}
#pragma endregion

#pragma region RichEditPara
	// RichEditPara
	RichEditPara::RichEditPara() :
		_autoWrapped(FALSE)
		, _needUpdateLayout(TRUE)
		, _simulateAlign(FALSE)
		, _breakAtTheEnd(TRUE)
		, _lineCount(1)
		, _initialized(FALSE)
		, _disableLayout(FALSE)
	{
		_isDirty = TRUE;
	}

	CRect RichEditPara::GetRect()
	{
		if (_isDirty)
		{
			_isDirty = FALSE;
			CalculateRect();
		}

		return _objRect;
	}

	BOOL RichEditPara::GetLineRect(int nLineNo, CRect& rcLine)
	{
		rcLine.SetRectEmpty();
// 		int ncpStart;
// 		int nLength;
// 		_pObjHost->SendMessage(EM_LINEINDEX, nLineNo, 0, (LRESULT*)&ncpStart);
// 		_pObjHost->SendMessage(EM_LINELENGTH, ncpStart, 0, (LRESULT*)&nLength);

		int ncpStart = (int)_pObjHost->SendMessage(EM_LINEINDEX, nLineNo, 0);
		int nLength = (int)_pObjHost->SendMessage(EM_LINELENGTH, ncpStart, 0);

		SComPtr<ITextRange>  spRangeLine;
		ITextDocument* pdoc = _pObjHost->GetTextDoc();
		pdoc->Range(ncpStart, ncpStart + nLength, &spRangeLine);
		if (!spRangeLine)
			return FALSE;

		CHARRANGE chr = { ncpStart, ncpStart + nLength };
		// http://technet.microsoft.com/zh-cn/hh768766(v=vs.90) �����Ͷ���
#define _tomClientCoord     256  // Ĭ�ϻ�ȡ��������Ļ���꣬ Use client coordinates instead of screen coordinates.
#define _tomAllowOffClient  512  // Allow points outside of the client area.
		long lTypeTopLeft = _tomAllowOffClient | _tomClientCoord | tomStart | TA_TOP | TA_LEFT;
		long lTypeRightBottom = _tomAllowOffClient | _tomClientCoord | tomEnd | TA_BOTTOM | TA_RIGHT;
		POINT   ptEnd, ptStart;
		spRangeLine->GetPoint(lTypeTopLeft, &ptStart.x, &ptStart.y);
		spRangeLine->GetPoint(lTypeRightBottom, &ptEnd.x, &ptEnd.y);
		rcLine.SetRect(ptStart, ptEnd);
		if (rcLine.Width() == 0)
			rcLine.right += 1;

		return TRUE;
	}

	BOOL RichEditPara::GetAutoWrapped()
	{
// 		int lineStart = 0;
// 		int lineEnd = 0;
// 		_pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMin, (LRESULT*)&lineStart);
// 		_pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMax, (LRESULT*)&lineEnd);
		int lineStart = (int)_pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMin);
		int lineEnd = (int)_pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMax);

		_autoWrapped = (lineEnd - lineStart + 1) > _lineCount;

		return _autoWrapped;
	}

	BOOL RichEditPara::CalculateRect()
	{
		CRect lineRect;
		_objRect.SetRectEmpty();

// 		int lineStart = 0;
// 		int lineEnd = 0;
// 		_pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMin, (LRESULT*)&lineStart);
// 		_pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMax, (LRESULT*)&lineEnd);

		int lineStart = _pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMin, 0);
		int lineEnd = _pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMax, 0);

		GetLineRect(lineStart, lineRect);
		_objRect.UnionRect(_objRect, lineRect);

		if (lineStart == lineEnd)
			return TRUE;

		GetLineRect(lineEnd, lineRect);
		_objRect.UnionRect(_objRect, lineRect);

		if (_autoWrapped)
		{
			CRect rcClient = _pObjHost->GetAdjustedRect();

			if (_alignType == RichEditObj::ALIGN_RIGHT)
			{
				_objRect.right = rcClient.right - _marginRect.left;
				_objRect.left = rcClient.left + _marginRect.right;
			}
			else
			{
				_objRect.right = rcClient.right - _marginRect.right;
				_objRect.left = rcClient.left + _marginRect.left;
			}

			return TRUE;
		}

		for (int n = lineStart + 1; n < lineEnd; ++n)
		{
			GetLineRect(n, lineRect);
			_objRect.UnionRect(_objRect, lineRect);
		}

		return TRUE;
	}

	BOOL RichEditPara::InsertIntoHost(IRichEditObjHost* pHost)
	{
		RichEditObj::InsertIntoHost(pHost);
		if (_breakAtTheEnd)
		{
			/*
			 * ������в����char range�������SetIndentsʱҪ���ϡ�
			 * ��Ҫ��Ϊ�˱�֤���������һ���ǿ���ʱ,����Ҳ����������
			*/

			pHost->SendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"\r\n");
		}
		// ����������
		RichEditObj* pChild = GetFirstChild();
		for (; pChild; pChild = pChild->GetNext())
		{
			if (pChild->IsClass(RichEditText::GetClassName()))
			{
				_lineCount += ((RichEditText*)pChild)->GetLineCount() - 1;
			}
		}

		return TRUE;
	}

	BOOL RichEditPara::IsWrapped()
	{
		int  nLineStart = 0;
		int  nLineEnd = 0;
		_pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMin, (LRESULT*)&nLineStart);
		_pObjHost->SendMessage(EM_EXLINEFROMCHAR, 0, _contentChr.cpMax, (LRESULT*)&nLineEnd);

		// �����ʾ��������ԭʼ����ʱ������Ҫ��,��Ϊ�Զ�������
		return (nLineEnd - nLineStart + 1) > _lineCount;
	}

	void RichEditPara::OffsetCharRange(int nOffset, BOOL bUpdate/*=FASLE*/)
	{
		RichEditObj::OffsetCharRange(nOffset);

		_needUpdateLayout = TRUE;

		if (bUpdate)
		{
			UpdatePosition();
		}
	}

	//
	// ����ʹ�ã�������һ��Ҫȷ��chr����ȷ
	// 
	void RichEditPara::SetCharRange(const CHARRANGE& chr)
	{
		_contentChr = chr;
		_needUpdateLayout = TRUE;
		UpdatePosition();
	}

	void RichEditPara::SetAlign(AlignType align)
	{
		if (align != _alignType)
			_needUpdateLayout = TRUE;

		RichEditObj::SetAlign(align);
	}

	void RichEditPara::UpdatePosition()
	{
		RichEditObj::UpdatePosition();

		if (_disableLayout)// �������Ҫ����,ֱ�ӷ���
			return;

		CComPtr<ITextPara>   ppara;
		CComPtr<ITextRange>  prange;
		ITextDocument* pdoc = _pObjHost->GetTextDoc();

		CHARRANGE chr = _contentChr;
		if (_breakAtTheEnd)
			chr.cpMax += 1;

		pdoc->Range(chr.cpMin, chr.cpMax, &prange);
		prange->GetPara(&ppara);

		if (!_initialized)
		{
			_initialized = TRUE;
			ppara->SetSpaceBefore(px2pt(_marginRect.top));
			ppara->SetSpaceAfter(px2pt(_marginRect.bottom));

			int nAlign = tomAlignLeft;

			if (!_simulateAlign)
			{
				if (_alignType == ALIGN_RIGHT)
					nAlign = tomAlignRight;
				else if (_alignType == ALIGN_CENTER)
					nAlign = tomAlignCenter;
			}

			ppara->SetAlignment(nAlign);
		}

		CRect rcAdjust = _pObjHost->GetAdjustedRect();
		int nLeftIndents = _alignType == ALIGN_RIGHT ? _marginRect.right : _marginRect.left;
		int nRightIndents = _alignType == ALIGN_RIGHT ? _marginRect.left : _marginRect.right;

		_autoWrapped = FALSE;
		//
		// ������Ҫʹ��ģ�����,������ٶ�
		// ��richedit�Լ��Ķ��뷽ʽ�����Ű�
		//
		if (!_simulateAlign || _alignType == ALIGN_LEFT)
		{
			GetAutoWrapped();

			if (_needUpdateLayout)
			{
				ppara->SetIndents(0, px2pt(nLeftIndents), px2pt(nRightIndents));
				_needUpdateLayout = FALSE;
			}

			return;
		}

		/* ģ������Ҷ���
		 * ע�⣺SetIndents��CalcParagraphRect�ܺ�ʱ,���Ծ�����ʹ��ģ����㿿��
		 *
		 * 1.�Ȱ���ߵ�������������
		 * 2.����cp��������rect
		 * 3.��richedit.width() - para.widht()����������,ģ���Ҷ���
		 * 4.���ݼ��������/������������
		*/
		// step 1
		ppara->SetIndents(0, px2pt(nLeftIndents), px2pt(nRightIndents));
		// step 2
		GetAutoWrapped();
		CalculateRect();
		if (_autoWrapped)
			return;
		// step 3
		if (_alignType == ALIGN_RIGHT)
			nLeftIndents = rcAdjust.Width() - _objRect.Width() - nRightIndents - 1; // ���ﲻ��1,����ͻ�����
		else
		{
			int mid = (rcAdjust.Width() - _objRect.Width() - nRightIndents - nLeftIndents) / 2 - 1;
			nLeftIndents = _marginRect.left + mid;
			nRightIndents = _marginRect.right + mid;
		}
		// step 4
		ppara->SetIndents(0, px2pt(nLeftIndents), px2pt(nRightIndents));
		_needUpdateLayout = FALSE;
		_isDirty = TRUE;
	}
#pragma endregion

#pragma region RichEditContent
	//RichEditContent
	void RichEditContent::UpdatePosition()
	{
		_isDirty = TRUE;
		_objRect.SetRect(0, 0, 0, 0);

		CRect rcHost = _pObjHost->GetAdjustedRect();
		AlignType type = rcHost.Width() > THRESHOLD_FOR_AUTOLAYOUT ? ALIGN_LEFT : ALIGN_RIGHT;

		if (_autoLayout)
		{
			SetAlign(type);
		}

		for (RichEditObj* p = _pFirstChild; p != NULL; p = p->GetNext())
		{
			p->UpdatePosition();
		}
	}

	CRect RichEditContent::GetRect()
	{
		if (_isDirty)
		{
			_isDirty = FALSE;
			_objRect.SetRect(0, 0, 0, 0);

			for (RichEditObj* p = _pFirstChild; p != NULL; p = p->GetNext())
			{
				CRect rcChild = p->GetRect();
				CRect rcTemp = _objRect;
				_objRect.UnionRect(rcChild, rcTemp);
			}
		}
		return _objRect;
	}

	BOOL RichEditContent::OnTimestampAttr(const SStringW& attr, BOOL bLoading)
	{
		_timestamp = _ttoi64(attr);
		return bLoading ? S_OK : S_FALSE;
	}
#pragma endregion


}