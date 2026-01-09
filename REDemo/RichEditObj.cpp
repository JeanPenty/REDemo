#include "stdafx.h"
#include "RichEditObj.h"

namespace SOUI
{
#define _tomClientCoord     256  // 默认获取到的是屏幕坐标， Use client coordinates instead of screen coordinates.
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
// 		for (pugi::xml_node xmlChild = xmlNode.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
// 		{
// 			if (xmlChild.type() != pugi::node_element) continue;
// 			RichEditObj* pChild = RichEditObjFactory::GetInstance().CreateObjectByName(xmlChild.name());
// 			if (pChild)
// 			{
// 				InsertChild(pChild);
// 				pChild->InitFromXml(xmlChild);
// 			}
// 		}

		return TRUE;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//RichEditContent
	///////////////////////////////////////////////////////////////////////////////////////////////////
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
}