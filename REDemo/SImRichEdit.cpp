#include "stdafx.h"
#include "souistd.h"
#include "SImRichEdit.h"
#include "RichEditObj.h"
#include "TOM.h"
#include <atlcomcli.h>
#include "interface\SRender-i.h"
#include "atl.mini\SComHelper.h"
#include <time.h>
#include "utils.h"
#include "gdialpha.h"
#include <RichOle.h>
#include "ClipboardConverter.h"
#include "RichEditOleBase.h"

#ifndef LY_PER_INCH
#define LY_PER_INCH 1440
#endif

//
// richedit�ڲ�����ļ���timer id
//
#define RETID_BGND_RECALC	0x01af
#define RETID_AUTOSCROLL	0x01b0
#define RETID_SMOOTHSCROLL	0x01b1
#define RETID_DRAGDROP		0x01b2
#define RETID_MAGELLANTRACK	0x01b3

	// richedit internal helpers

	/*
	* ��ȡָ��OLE��cp
	* @param pOle: ole����
	* @param iOle: richedit��ĵڼ���ole
	* @return: ole��cp
	*/
LONG GetOleCP(IRichEditOle* pOle, int iOle)
{
	REOBJECT reobj = { 0 };
	reobj.cbStruct = sizeof(REOBJECT);
	pOle->GetObject(iOle, &reobj, REO_GETOBJ_NO_INTERFACES);
	return reobj.cp;
}

/*
 * �����۰���ҵķ�����[cpMin,cpMax)���ҳ���һ���ɼ���OLE�±�
 *
 * @param pOle: richedit��OLE����
 * @param iBegin: ���ҵ���ʼλ��
 * @param iEnd: ���ҵĽ���λ��
 * @param cpMin: richedit��һ���ɼ��ַ���λ��
 * @param cpMax: richedit���һ���ɼ��ַ���λ��
 *
 * @return: ����ҵ���һ���ɼ���ole�򷵻�ole��cp�����򷵻�-1.
 */
int FindFirstOleInrange(IRichEditOle* pOle, int iBegin, int iEnd, int cpMin, int cpMax)
{
	if (iBegin == iEnd) return -1;

	int iMid = (iBegin + iEnd) / 2;

	LONG cp = GetOleCP(pOle, iMid);

	if (cp < cpMin)
	{
		return FindFirstOleInrange(pOle, iMid + 1, iEnd, cpMin, cpMax);
	}
	else if (cp >= cpMax)
	{
		return FindFirstOleInrange(pOle, iBegin, iMid, cpMin, cpMax);
	}
	else
	{
		int iRet = iMid;
		while (iRet > iBegin)
		{
			cp = GetOleCP(pOle, iRet - 1);
			if (cp < cpMin) break;
			iRet--;
		}
		return iRet;
	}
}

/*
 * �����۰���ҵķ�����[cpMin,cpMax)���ҳ����һ���ɼ���OLE�±�
 *
 * @param pOle: richedit��OLE����
 * @param iBegin: ���ҵ���ʼλ��
 * @param iEnd: ���ҵĽ���λ��
 * @param cpMin: richedit��һ���ɼ��ַ���λ��
 * @param cpMax: richedit���һ���ɼ��ַ���λ��
 *
 * @return: ����ҵ����һ���ɼ���ole�򷵻�ole��cp�����򷵻�-1.
 */
int FindLastOleInrange(IRichEditOle* pOle, int iBegin, int iEnd, int cpMin, int cpMax)
{
	if (iBegin == iEnd) return -1;

	int iMid = (iBegin + iEnd) / 2;

	LONG cp = GetOleCP(pOle, iMid);

	if (cp < cpMin)
	{
		return FindLastOleInrange(pOle, iMid + 1, iEnd, cpMin, cpMax);
	}
	else if (cp >= cpMax)
	{
		return FindLastOleInrange(pOle, iBegin, iMid, cpMin, cpMax);
	}
	else
	{
		int iRet = iMid;
		while (iRet < (iEnd - 1))
		{
			cp = GetOleCP(pOle, iRet + 1);
			if (cp >= cpMax) break;
			iRet++;
		}
		return iRet;
	}
}

// impl RichEditDropTarget
class RichEditDropTarget : public IDropTarget
{
public:
	RichEditDropTarget(IRichEditObjHost* phost)
		:_ref(1)
		, _pHost(phost)
	{
		SASSERT(_pserv);
		_pserv = phost->GetTextServ();
		_pserv->AddRef();
	}

	~RichEditDropTarget()
	{
		SASSERT(_pserv);
		_pserv->Release();
	}

	//IUnkown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void __RPC_FAR* __RPC_FAR* ppvObject)
	{
		HRESULT hr = E_NOINTERFACE;
		if (riid == __uuidof(IUnknown))
			*ppvObject = (IUnknown*)this, hr = S_OK;
		else if (riid == __uuidof(IDropTarget))
			*ppvObject = (IDropTarget*)this, hr = S_OK;
		if (SUCCEEDED(hr)) AddRef();
		return hr;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) { return ++_ref; }

	virtual ULONG STDMETHODCALLTYPE Release(void) {
		ULONG uRet = --_ref;
		if (uRet == 0) delete this;
		return uRet;
	}

	//IDropTarget
	virtual HRESULT STDMETHODCALLTYPE DragEnter(
		/* [unique][in] */ IDataObject* pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ DWORD* pdwEffect)
	{
		HRESULT hr = S_FALSE;
		IDropTarget* pDropTarget = NULL;
		hr = _pserv->TxGetDropTarget(&pDropTarget);
		if (SUCCEEDED(hr))
		{
			hr = pDropTarget->DragEnter(pDataObj, grfKeyState, pt, pdwEffect);
			*pdwEffect = DROPEFFECT_COPY;
			pDropTarget->Release();
		}
		return hr;
	}

	virtual HRESULT STDMETHODCALLTYPE DragLeave(void)
	{
		HRESULT hr = S_FALSE;
		IDropTarget* pDropTarget = NULL;
		hr = _pserv->TxGetDropTarget(&pDropTarget);
		if (SUCCEEDED(hr))
		{
			hr = pDropTarget->DragLeave();
			pDropTarget->Release();
		}
		return hr;
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver(
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ DWORD* pdwEffect)
	{
		/*
		* ������DragOver��richedit������Ϊrichedit��DragOverʱ�ᶯ̬����
		* ��������λ�ã�����ҳ����ܻ��Զ�����/���¹���
		*/
		*pdwEffect = DROPEFFECT_COPY;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Drop(
		/* [unique][in] */ IDataObject* pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ DWORD* pdwEffect)
	{
		if (*pdwEffect == DROPEFFECT_NONE)
		{
			return S_FALSE;
		}

		if (!_pHost->IsEditable())
		{
			/*
			* ������drop��readonly״̬��richedit������Ϊ��ϣ���ƻ�richedit����ѡ״̬��
			*/
			RichFormatConv conv;
			if (conv.InitFromDataObject(pDataObj) != 0)
			{
				_pHost->AcceptContent(&conv);
			}

			*pdwEffect = DROPEFFECT_COPY;
			return DragLeave();
		}

		HRESULT hr = S_FALSE;
		IDropTarget* pDropTarget = NULL;
		hr = _pserv->TxGetDropTarget(&pDropTarget);
		if (SUCCEEDED(hr))
		{
			hr = pDropTarget->Drop(pDataObj, grfKeyState, pt, pdwEffect);
			pDropTarget->Release();
		}

		return hr;
	}

protected:

	IRichEditObjHost* _pHost;
	ITextServices* _pserv;            // pointer to Text Services object
	LONG              _ref;
};

SImRichEdit::SImRichEdit() : _pTextDoc(NULL)
, _pLastHoverObj(NULL)
, _isBackgroundDirty(TRUE)
, _lastDrawTicks(0)
, _isDrawEnable(TRUE)
, _scrollbarAtBottom(TRUE)
, _scrollbarOutdated(FALSE)
, _isDragging(FALSE)
, _readOnlyBeforeDrag(FALSE)
, _fixVScroll(TRUE)
, _isCreatIme(false)
{
// 	m_evtSet.addEvent(EVENTID(EventQueryAccept));
// 	m_evtSet.addEvent(EVENTID(EventRichEditObj));
// 	m_evtSet.addEvent(EVENTID(EventRichEditScroll));
}

SImRichEdit::~SImRichEdit()
{
	GetContainer()->UnregisterTimelineHandler(this);
}

ISwndContainer* SImRichEdit::GetHostContainer()
{
	return GetContainer();
}

CRect SImRichEdit::GetHostRect()
{
	return GetClientRect();
}

CRect SImRichEdit::GetAdjustedRect()
{
	CRect rc;
	rc = GetClientRect();
	rc.DeflateRect(m_rcInset);

	return rc;
}

int SImRichEdit::GetCharCount()
{
	SComPtr<ITextRange> range2;
	_pTextDoc->Range((~(ULONG)0) / 2, (~(ULONG)0) / 2, &range2);
	long end;
	range2->GetEnd(&end);
	return end;
}

void SImRichEdit::DirectDraw(const CRect& rc)
{
    if (GetState() & WndState_Invisible) // ��������&&,������IsVisible���ж�
        return;

    if (!_isDrawEnable || !_pBackgroundRt || _isBackgroundDirty)
        return;

    CRect rcClient = GetClientRect();
    rcClient.IntersectRect(rcClient, rc);
    if (rcClient.IsRectNull() || !_pBackgroundRt)
        return;

    CAutoRefPtr<IRegionS> rgn;
    GETRENDERFACTORY->CreateRegion(&rgn);
    rgn->CombineRect(rcClient, RGN_OR);

    /*
    * ���������ַ���ȥ������
    * - 1.���ô��ڵ�GetRenderTarget���Ƚ����ڣ��������richedit��Ƕ�׵Ĳ�ν��Ч�ʾͱȽ���
    * - 2.�Լ�ά�����汳������onsize��ʱ��ȥ���±������ô����ٶȽϿ죬���Ǳ����п����ǲ�׼ȷ�ġ�
    *
    * �����ֻ������Ϳ�����ˣ���������Ǵ�ɫ�ģ��Ǿ��õ�2�ֿ���
    *
    * =============================================================================
    * ���ڴ���gif��ˢ�£�Ҳ�����ַ���
    * - 1.����richedit��Draw�¼�����richeditȥ����ole�Ļ����¼���
    *     ���ַ����ȽϷ��㣬��Ϊ��richeditȥ�����ģ�����Ҫ����ʲôλ�á�
    *     ѡ��״̬��������⣬����Ч�ʽϵ͡�
    *
    * - 2.ֱ�Ӵ�һ��dc����renderTarget��ole��������ȥ�������
    *     ���������ϡ����ַ���Ч�ʽϸߣ�������Ҫ�Լ������Ƿ�ѡ��
    *     ��״̬���Ƚ��鷳��SoSmiley���������ַ�ʽ��
    *
    * �����õ��ǵ�1��ȥ����gifˢ��,Ч����ǿ�����Խ��ܣ��������Ҫ�ٸĳɵ�2�֡�
    */

    /*
    * ����1������
    */
    //IRenderTarget *pRT = GetRenderTarget(OLEDC_PAINTBKGND,rgn);
    //SSendMessage(WM_ERASEBKGND,(WPARAM)pRT);
    //RedrawRegion(pRT, rgn);
    //ReleaseRenderTarget(pRT);

    /*
    * ����2������
    */
    //TODO:
    int n = GetTickCount();
    CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(rcClient);
    pRT->BitBlt(rcClient, _pBackgroundRt, rcClient.left, rcClient.top);
    int n1 = GetTickCount() - n;

    n = GetTickCount();
    RedrawRegion(pRT, rgn);
    ReleaseRenderTarget(pRT);
    int n2 = GetTickCount() - n;

    //STRACE(L"direct draw:%d, n1:%d, n2:%d", GetTickCount(), n1, n2);
}

void SImRichEdit::DelayDraw(const CRect& rc)
{
	if (!_pDelayDrawRgn)
		GETRENDERFACTORY->CreateRegion(&_pDelayDrawRgn);

	_pDelayDrawRgn->CombineRect(rc, RGN_OR);
}

HRESULT SImRichEdit::SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pRet/* = NULL*/)
{
	LRESULT lRet = SSendMessage(uMsg, wParam, lParam);
	if (pRet) *pRet = lRet;
	return S_OK;
}

ITextDocument* SImRichEdit::GetTextDoc()
{
    return _pTextDoc;
}

ITextServices* SImRichEdit::GetTextServ()
{
    return GetTextServices();
}

BOOL SImRichEdit::AcceptContent(RichFormatConv* conv)
{
// 	EventQueryAccept evt(this);
// 	evt.Conv = conv;
// 	return FireEvent(evt);

    return TRUE;
}

bool SImRichEdit::NotifyRichObjEvent(RichEditObj* obj, int subEvent, WPARAM wParam, LPARAM lParam)
{
// 	EventRichEditObj evt(this);
// 	evt.RichObj = obj;
// 	evt.SubEventId = subEvent;
// 	evt.wParam = wParam;
// 	evt.lParam = lParam;
// 
// 	return !!FireEvent(evt);

    return true;
}

void SImRichEdit::EnableDraw(BOOL bEnable)
{
    _isDrawEnable = bEnable;
}

void SImRichEdit::UpdateRichObj(RichEditObj* obj)
{
	_scrollbarOutdated = TRUE;

 	UpdateContext ctx;
 	PreUpdate(ctx);
 	obj->UpdatePosition();
 	CRect rc = obj->GetRect();
 	InvalidateRect(rc);
 	DoneUpdate(ctx);
}

int SImRichEdit::GetRemainingLength()
{
    return GetLimitText();
}

BOOL SImRichEdit::IsEditable()
{
    return !GetReadOnly() && !_readOnlyBeforeDrag;
}

void SImRichEdit::Activate()
{
    SetFocus();
}

/*
* �������ݵ�richedit
* @param lpszContent: XML��ʽ������
* @param uInsertAt:   �����λ�ã���������ȡֵ��Χ֮�⣬������ȡ��������ֵ
*                     - RECONTENT_LAST   �� ���뵽���
*                     - RECONTENT_CARET  �� ���뵽��괦
*
* @return UINT: ��Ϣʵ�ʲ����λ��
*/
UINT SImRichEdit::InsertContent(LPCWSTR lpszContent, UINT uInsertAt/* = RECONTENT_LAST*/)
{
	RichEditContent* p = CreateRichEditConent(lpszContent);
	if (p == NULL)
		return -1;
    CHARRANGE chr = { -1, -1 };
    if (uInsertAt == RECONTENT_CARET)
        _richContents.Add(p);
    else if (uInsertAt >= _richContents.GetCount())
    {
        uInsertAt = _richContents.GetCount();
        if (uInsertAt > 0)
        {
            chr = _richContents.GetAt(uInsertAt - 1)->GetCharRange();
            SetSel(chr.cpMax, chr.cpMax);
        }
        _richContents.Add(p);
    }
    else
    {
		chr = _richContents.GetAt(uInsertAt)->GetCharRange();
		SetSel(chr.cpMin, chr.cpMin);
		_richContents.InsertAt(uInsertAt, p);
    }

	UpdateContext ctx;
	PreUpdate(ctx);

	p->InsertIntoHost(this);
	p->UpdatePosition();
	chr = p->GetCharRange();
	ReLocateContents(uInsertAt, chr.cpMax - chr.cpMin);
	InvalidateRect(p->GetRect());

	if (_isDragging)
	{
		// �������ק״̬�²��������ݣ���ʱ��������SetReadOnly(TRUE)�� ����������
		// ����ԭ���OnMouseMove��ע�͡�
		// Ӧ�õȵ���ק������(Ҳ������__super::OnMouseMove����֮��)������ֻ������
		ctx.bReadOnly = FALSE;
	}

	DoneUpdate(ctx);
	UpdateVisibleCharRanges();
    return uInsertAt;
}

void SImRichEdit::DeleteContent(UINT uIndex)
{
	if (uIndex >= _richContents.GetCount())
		return;

	RichEditObj* pobj = _pLastHoverObj;
	RichEditContent* prec = _richContents.GetAt(uIndex);
	CHARRANGE chr = prec->GetCharRange();
	UINT index = 0;
	for (; pobj != NULL; pobj = pobj->GetParent(), ++index)
	{
		if (prec == pobj)
		{
			_pLastHoverObj = NULL;
			break;
		}
	}

	UpdateContext ctx;
	PreUpdate(ctx);

	SetSel(chr.cpMin, chr.cpMax);
	SSendMessage(EM_REPLACESEL, TRUE, (LPARAM)(LPCWSTR)L"");
	ReLocateContents(uIndex, chr.cpMin - chr.cpMax);
	_richContents.RemoveAt(uIndex);

	DoneUpdate(ctx);
	delete prec;
}

UINT SImRichEdit::GetContentCount()
{
	return _richContents.GetCount();
}

SStringW SImRichEdit::GetSelectedContent(CHARRANGE* lpchrg/* = NULL*/)
{
	SASSERT(lpchrg != NULL);

	CHARRANGE selChr = *lpchrg;
	selChr.cpMin = (selChr.cpMin < 0) ? 0 : selChr.cpMin;
	selChr.cpMax = (selChr.cpMax == -1) ? GetCharCount() : selChr.cpMax;

	SStringW subText;
	SStringW content = L"<RichEditContent>";

	SComPtr<IRichEditOle> ole;
	SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);
	int oleCount = ole->GetObjectCount();

	int oleCp = 0;
	for (int i = 0; i < oleCount && oleCp < selChr.cpMax; ++i)
	{
		REOBJECT reobj = { 0 };
		reobj.cbStruct = sizeof(REOBJECT);

		if (FAILED(ole->GetObject(i, &reobj, REO_GETOBJ_POLEOBJ)))
		{
			break;
		}

		oleCp = reobj.cp;
		if (reobj.cp < selChr.cpMin || reobj.cp >= selChr.cpMax)
		{
			reobj.poleobj->Release();
			continue;
		}

		if (selChr.cpMin < reobj.cp)
		{
			CHARRANGE chr = { selChr.cpMin, reobj.cp };
			GetRangeText(chr, subText);
			content += RichEditText::MakeFormatedText(subText);
		}

		RichEditOleAdapter* pOle = static_cast<RichEditOleAdapter*>(reobj.poleobj);
		content += pOle->_pHost->GetSelFormatedText();

		reobj.poleobj->Release();
		selChr.cpMin = reobj.cp + 1;
	}

	if (selChr.cpMin < selChr.cpMax)
	{
		CHARRANGE chr = { selChr.cpMin, selChr.cpMax };
		GetRangeText(chr, subText);
		content += RichEditText::MakeFormatedText(subText);
	}

	content += L"</RichEditContent>";
	return content;
}

void SImRichEdit::Clear()
{
	SelectAll();
	SSendMessage(EM_REPLACESEL, FALSE, (LPARAM)L"", 0);
	EmptyUndoBuffer();

	for (size_t i = 0; i < _richContents.GetCount(); ++i)
	{
		RichEditContent* p = _richContents.GetAt(i);
		p->Release();
	}

	_richContents.RemoveAll();
	_pLastHoverObj = NULL;
}

void SImRichEdit::ScrollToBottom()
{
	SSendMessage(WM_VSCROLL, (WPARAM)SB_BOTTOM, NULL);
}

BOOL SImRichEdit::IsScrollAtTop()
{
	CRect rcTrack = GetScrollBarRect(TRUE);
	return rcTrack.top == 0;
}

BOOL SImRichEdit::IsScrollAtBottom()
{
	return _scrollbarAtBottom;
}


RichEditObj* SImRichEdit::GetElementById(LPCWSTR lpszId)
{
	for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
	{
		RichEditObj* pObjRet = _richContents.GetAt(npos)->GetById(lpszId);
		if (pObjRet != NULL)
			return pObjRet;
	}

	return NULL;
}

RichEditObj* SImRichEdit::HitTest(POINT ptInControl)
{
	for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
	{
		RichEditObj* pObjHitted = HitTest(_richContents.GetAt(npos), ptInControl);
		if (pObjHitted != NULL)
			return pObjHitted;
	}

	return NULL;
}

RichEditContent* SImRichEdit::GetContent(LPCWSTR pszId)
{
	if (!pszId)
		return NULL;

	for (size_t n = 0; n < _richContents.GetCount(); ++n)
	{
		RichEditContent* p = _richContents[n];
		if (p->GetId() == pszId)
			return p;
	}

	return NULL;
}

RichEditContent* SImRichEdit::GetContent(UINT uIndex)
{
	if (uIndex >= _richContents.GetCount())
		return NULL;

	return _richContents.GetAt(uIndex);
}

RichEditOleBase* SImRichEdit::GetOleById(LPCWSTR lpszId)
{
	RichEditObj* pObj = GetElementById(lpszId);
	SStringW cn = pObj->GetClassName();
	RichEditOleBase * pOle = dynamic_cast<RichEditOleBase*>(pObj);


 	return pOle;
}

/*
 * ��ȡָ���ַ�λ�õ�ole
 * @param cp: �ַ�λ�á����<0�����Զ���ȡ��ǰѡ�����ݵ��ַ�λ�á����ѡ������>1������NULL��
 * return: ��Ӧ��oleָ�룬���������cp����ole������NULL
 */
RichEditOleBase* SImRichEdit::GetSelectedOle(int cp/* = -1*/)
{
	RichEditOleBase* pOleObject = NULL;

	SComPtr<IRichEditOle> ole;
	SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);

	if (cp < 0)
	{
		CHARRANGE chrSel;
		GetSel(&chrSel.cpMin, &chrSel.cpMax);
		if (chrSel.cpMax - chrSel.cpMin != 1)
		{
			return NULL;
		}

		cp = chrSel.cpMin;
	}

	REOBJECT reo = { 0 };
	reo.cp = cp;
	reo.cbStruct = sizeof(REOBJECT);

	if (ole->GetObject(REO_IOB_USE_CP, &reo, REO_GETOBJ_POLEOBJ) == S_OK)
	{
		pOleObject = dynamic_cast<RichEditOleBase*>(reo.poleobj);
		if (!pOleObject)
		{
			//
			// ��������Ҫ����Release
			// ����ֻ�Ƿ�ֹpOleObjectΪ��ʱ�������޷�����Release�����ı���������й©
			//
			reo.poleobj->Release();
		}
	}

	return pOleObject;
}

UINT SImRichEdit::DeleteContent(RichEditContent* prec)
{
	if (!prec)
		return _richContents.GetCount();

	for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
	{
		RichEditContent* p = _richContents.GetAt(npos);

		if (p == prec)
		{
			DeleteContent(npos);
			return npos;
		}
	}

	return _richContents.GetCount();
}

/*
 * ��ȡ����λ�ã�λ�����������Ļ�����Ͻǡ�
 * λ����ʵʱ����ģ�Ч�ʽ�GetCaretRect2�ͣ�ͨ��������þ�ȷ�Ĺ��λ�á�
 *
 * @param rcCursor: ����Ĺ��λ��
 * @return BOOL: TRUE
 */
BOOL SImRichEdit::GetCaretRect(CRect& rcCaret)
{
	LONG cpStart = -1;
	LONG cpEnd = -1;

	GetSel(&cpStart, &cpEnd);
	rcCaret.SetRectEmpty();

	if (cpStart < 0 || cpEnd < 0)
		return FALSE;

	SComPtr<ITextRange> prange;
	_pTextDoc->Range(cpStart, cpEnd, &prange);

	if (!prange)
		return FALSE;

	// http://technet.microsoft.com/zh-cn/hh768766(v=vs.90) �����Ͷ���
#define _tomClientCoord     256  // Ĭ�ϻ�ȡ��������Ļ���꣬ Use client coordinates instead of screen coordinates.
#define _tomAllowOffClient  512  // Allow points outside of the client area.

	POINT ptStart = { 0,0 };
	POINT ptEnd = { 0,0 };
	long lTypeTopLeft = _tomAllowOffClient | _tomClientCoord | tomStart | TA_TOP | TA_LEFT;
	long lTypeRightBottom = _tomAllowOffClient | _tomClientCoord | tomEnd | TA_BOTTOM | TA_RIGHT;
	if (prange->GetPoint(lTypeTopLeft, &ptStart.x, &ptStart.y) != S_OK ||
		prange->GetPoint(lTypeRightBottom, &ptEnd.x, &ptEnd.y) != S_OK)
		return FALSE;

	rcCaret.SetRect(ptStart, ptEnd);
	if (rcCaret.Width() == 0)
		rcCaret.right += 1;

	LPRECT prc = (LPRECT)rcCaret;
	HWND hwnd = GetContainer()->GetHostHwnd();

	::ClientToScreen(hwnd, (LPPOINT)prc);
	::ClientToScreen(hwnd, ((LPPOINT)prc) + 1);
	return TRUE;
}

/*
 * ��ȡ����λ�ã�λ���������richedit�����Ͻǡ�
 * λ�ò�����ʵʱ�ģ�ͨ�����������õ����߶ȡ�
 *
 * @param rcCursor: ����Ĺ��λ��
 * @return BOOL: TRUE
 */
BOOL SImRichEdit::GetCaretRect2(CRect& rcCaret)
{
	rcCaret = caretRect;
	return TRUE;
}

BOOL SImRichEdit::CanPaste()
{
	RichFormatConv::ClipboardFmts fmts;
	RichFormatConv conv;

	conv.GetSupportedFormatsFromClipboard(fmts);
	for (UINT i = 0; i < fmts.GetCount(); ++i)
	{
		if (IsClipboardFormatAvailable(fmts[i]))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * ���±������棬ͨ����OnSize���øú������ú���ֻ���޸ı�������Ĵ�С��
 * ��OnSize�����GetContainer()->OnGetRenderTarget�������⣬���Ա����Ļ��ƹ�������OnPaint
 */
void SImRichEdit::UpdateBkgndRenderTarget()
{
	// ���±���RenderTarget
	CRect rcWnd = GetClientRect();
	if (!_pBackgroundRt)
	{
		GETRENDERFACTORY->CreateRenderTarget(&_pBackgroundRt, rcWnd.Width(), rcWnd.Height());
	}
	else
	{
		_pBackgroundRt->Resize(rcWnd.Size());
	}
	_pBackgroundRt->SetViewportOrg(-rcWnd.TopLeft());
	_isBackgroundDirty = TRUE;


	if (_pDelayDrawRgn)
	{
		//STRACE(L"size changed, clear delay draw rgn");
		_pDelayDrawRgn->Clear();
	}
}

void SImRichEdit::MarkBkgndRtDirty()
{
	_isBackgroundDirty = TRUE;
}

void SImRichEdit::SetAutoFixVScroll(BOOL fix)
{
	_fixVScroll = fix;
}

/*
 * richedit��һ��RETID_BGND_RECALC��ʱ���������ӳ��Ű档
 * richedit�ڲ����Ҫ�Ű��������һ����ʱ�����ڻ�û���������ʱ��֮ǰ�������Ҫ�Ű棬��ô���������ö�ʱ����
 * �����Ϳ��Ա���̫Ƶ�����Ű浼��Ч�ʵ��¡�
 *��
 * �ú���������richedit���������Ű�Ķ�ʱ����ǿ����richedit�������Ű�
 */
void SImRichEdit::ForceUpdateLayout()
{
	OnTimer(RETID_BGND_RECALC);
}

/*
 * ���¼���content��charrange��ͨ����conetent�б��м���롢ɾ����ĳ��contentʱ��
 * ��Ҫ���¼����޸�λ�����µ�conent charrange��
 *
 * @param start: ��Ҫ����content����ʼλ��
 * @param offset: content charrange��ƫ�ơ��п����Ǹ���
 */
void SImRichEdit::ReLocateContents(UINT start, int offset)
{
	if (start >= _richContents.GetCount())
		return;

	start += 1;
	for (; start < _richContents.GetCount(); ++start)
	{
		RichEditContent* pCurr = _richContents.GetAt(start);
		pCurr->OffsetCharRange(offset);
	}
}

BOOL SImRichEdit::GetContentIndex(LPCWSTR pszId, UINT& index)
{
	for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
	{
		RichEditContent* p = _richContents.GetAt(npos);

		if (p->GetId() == pszId)
		{
			index = npos;
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * ��ָ�����±꿪ʼ����content��λ��
 * @param start: ��ʼ����content���±�
 * @param end:   �������µ�content�±�
 *
 * ���·�Χ��[start, end)
 */
void SImRichEdit::UpdateContentPosition(UINT start, UINT end/* = RECONTENT_LAST*/)
{
	_scrollbarOutdated = TRUE;
	UpdateContext ctx;
	PreUpdate(ctx);

	for (size_t n = start; n < end && n < _richContents.GetCount(); ++n)
		_richContents.GetAt(n)->UpdatePosition();

	ctx.bHasScrollBar = HasScrollBar(TRUE); //ǿ�Ʋ�ˢ������
	DoneUpdate(ctx);

	UpdateVisibleCharRanges();
}

void SImRichEdit::ScrollTo(LPCWSTR pszId)
{
	for (UINT i = 0; i < _richContents.GetCount(); ++i)
	{
		RichEditContent* p = _richContents.GetAt(i);

		if (p->GetId() == pszId)
		{
			ScrollTo(i);
			return;
		}
	}
}

void SImRichEdit::ScrollTo(UINT uIndex)
{
	if (uIndex >= _richContents.GetCount())
		return;

	RichEditContent* pc = _richContents.GetAt(uIndex);
	CHARRANGE chr = pc->GetCharRange();

	SetSel(chr.cpMin, chr.cpMin);
	SetSel(-1, 0);
}

void SImRichEdit::SetFontSize(int size)
{
	CHARFORMAT cf;
	GetDefCharFormat(cf);

	cf.dwMask = cf.dwMask | CFM_SIZE;
	cf.yHeight = size * 20;

	SetDefCharFormat(cf);
}

void SImRichEdit::SetSelectionColor(COLORREF cr)
{
	CHARFORMAT cf;
	GetDefCharFormat(cf);
	cf.dwMask |= CFM_COLOR;
	cf.crTextColor = cr;

	SSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void SImRichEdit::GetSel(long* pStartCp, long* pEndCp)
{
	CHARRANGE cr = { 0, 0 };
	SSendMessage(EM_EXGETSEL, NULL, (LPARAM)&cr);

	*pStartCp = cr.cpMin;
	*pEndCp = cr.cpMax;
}

void SImRichEdit::SetSel(int nStart, int nEnd)
{
	CHARRANGE cr = { nStart, nEnd };
	//SSendMessage(EM_EXSETSEL, NULL, (LPARAM)&cr);
	SSendMessage(EM_SETSEL, nStart, nEnd);
}

BOOL SImRichEdit::SetParaFormat(PARAFORMAT2& pf)
{
	pf.cbSize = sizeof(PARAFORMAT2);
	SSendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf);
	return TRUE;
}

int SImRichEdit::LineFromChar(int ncp)
{
	return (int)SSendMessage(EM_LINEFROMCHAR, ncp, 0);
}

int SImRichEdit::LineIndex(int nLineNum)
{
	return SSendMessage(EM_LINEINDEX, nLineNum, 0);
}

int SImRichEdit::LineLength(int nLineCP /* = -1 */)
{
	return (int)SSendMessage(EM_LINELENGTH, nLineCP, 0);
}

void SImRichEdit::PosFromChar(UINT ncp, POINT* pPos)
{
	POINTL pt = { 0, 0 };
	SSendMessage(EM_POSFROMCHAR, (WPARAM)&pt, ncp);
	pPos->x = pt.x;
	pPos->y = pt.y;
}

int SImRichEdit::CharFromPos(POINT pt)
{
	POINTL  ptl = { pt.x, pt.y };
	return (int)SSendMessage(EM_CHARFROMPOS, 0, (LPARAM)&ptl);
}

int SImRichEdit::GetFirstVisibleLine()
{
	return (int)SSendMessage(EM_GETFIRSTVISIBLELINE, 0, 0);
}

int SImRichEdit::GetRangeText(const CHARRANGE& chr, SStringW& strText)
{
	TEXTRANGE txtrg = { chr, NULL };
	strText.Empty();

	if (chr.cpMax <= chr.cpMin)
		return 0;

	txtrg.lpstrText = new WCHAR[chr.cpMax - chr.cpMin + 1];
	memset(txtrg.lpstrText, 0, chr.cpMax - chr.cpMin + 1);
	int nTextLen = SSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&txtrg);
	if (nTextLen > 0)
		strText = txtrg.lpstrText;

	delete txtrg.lpstrText;
	return nTextLen;

}

void SImRichEdit::SetAutoVScroll(BOOL bEnabled)
{
	if (bEnabled)
		m_dwStyle |= ES_AUTOVSCROLL;
	else
		m_dwStyle &= ~ES_AUTOVSCROLL;

	GetTextServices()->OnTxPropertyBitsChange(TXTBIT_SCROLLBARCHANGE, TXTBIT_SCROLLBARCHANGE);
}

BOOL SImRichEdit::IsAutoVScroll()
{
	return (m_dwStyle & ES_AUTOVSCROLL) != 0;
}

void SImRichEdit::SelectAll()
{
	CHARRANGE chr = { 0, -1 };
	SSendMessage(EM_EXSETSEL, 0, (LPARAM)&chr);
}

void SImRichEdit::Copy()
{
	SSendMessage(WM_COPY, 0, 0);
}

void SImRichEdit::Cut()
{
	SSendMessage(WM_CUT, 0, 0);
}

void SImRichEdit::Paste()
{
	REPASTESPECIAL  reps = { 0, (DWORD_PTR)0 };
	SSendMessage(EM_PASTESPECIAL, CF_TEXT, (LPARAM)&reps, NULL);
}

void SImRichEdit::EmptyUndoBuffer()
{
	SSendMessage(EM_EMPTYUNDOBUFFER, 0, 0, NULL);
}

BOOL SImRichEdit::GetDefCharFormat(CHARFORMAT& cf)
{
	cf.cbSize = sizeof(CHARFORMAT);
	SSendMessage(EM_GETCHARFORMAT, 0, (LPARAM)&cf);
	return TRUE;
}

BOOL SImRichEdit::SetDefCharFormat(CHARFORMAT& cf)
{
	cf.cbSize = sizeof(CHARFORMAT);
	SSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
	return TRUE;
}

/*
 * ������꣬��richedit����
 * @param pBmp:    ���λͼ
 * @param nWid:    ������
 * @param nHeight: ���߶�
 * @return BOOL:   TRUE:�����ɹ�,FASLE:����ʧ��
 *
 * note: ����ѡ������ʱ��richedit���ݹ����Ŀ�������Ϣ��pBmp��Я���Ŀ�����ʱ�򲢲�һ��
 */
BOOL SImRichEdit::CreateCaret(HBITMAP pBmp, int nWid, int nHeight)
{
	caretRect.right = caretRect.left + nWid;
	caretRect.bottom = caretRect.top + nHeight;
	return SRichEdit::CreateCaret(pBmp, nWid, nHeight);
}

/*
 * ���ù��λ�ã���richedit����
 */
void SImRichEdit::SetCaretPos(int x, int y)
{
	caretRect.left = x;
	caretRect.top = y;
	__super::SetCaretPos(x, y);
}

BOOL SImRichEdit::IsRenderTargetEmpty(IRenderTarget* pRt)
{
	if (!pRt)
		return TRUE;

	IBitmapS* bmp = (IBitmapS*)pRt->GetCurrentObject(OT_BITMAP);
	LPBYTE lpBits = (LPBYTE)bmp->LockPixelBits();
	if (!lpBits)
		return TRUE;

	BYTE r = *lpBits++;
	BYTE g = *lpBits++;
	BYTE b = *lpBits++;
	BYTE a = *lpBits++;

	return a == 0;
}

void SImRichEdit::EnableDragDrop(BOOL enable)
{
	RevokeDragDrop((HWND)m_swnd);
	if (enable)
	{
// 		RichEditDropTarget* pdt = new RichEditDropTarget(this);
// 		GetContainer()->RegisterDragDrop(m_swnd, pdt);
// 		pdt->Release();
	}
}

RichEditObj* SImRichEdit::HitTest(RichEditObj* pObject, POINT ptInControl)
{
	for (; pObject != NULL; pObject = pObject->GetNext())
	{
		if (pObject->GetHitTestable() && pObject->PointInObject(ptInControl))
			return pObject;

		RichEditObj* p = HitTest(pObject->GetFirstChild(), ptInControl);
		if (p != NULL)
			return p;
	}

	return NULL;
}

RichEditContent* SImRichEdit::CreateRichEditConent(LPCWSTR lpszContent)
{
	if (lpszContent == NULL)
		return NULL;

	RichEditContent* pRet = NULL;
	unsigned int flag = pugi::parse_cdata | pugi::parse_escapes | pugi::parse_eol;
	pugi::xml_document  doc;
	int status = doc.load_buffer(lpszContent, wcslen(lpszContent) * sizeof(WCHAR), flag);
	if (doc.load_buffer(lpszContent, wcslen(lpszContent) * sizeof(WCHAR), flag))
	{
 		pRet = new RichEditContent();
// 		if (!pRet->InitFromXml(doc.child(RichEditContent::GetClassName())))
// 		{
// 			delete pRet;
// 			return NULL;
// 		}

		SXmlNode xmlNode = doc.root().first_child();
		if (!pRet->InitFromXml(&xmlNode))
		{
			delete pRet;
			return NULL;
		}
	}

	return pRet;
}

void SImRichEdit::PreUpdate(UpdateContext& context)
{
	context.bReadOnly = GetReadOnly();
	if (context.bReadOnly)
		SetReadOnly(FALSE);

	context.bHasScrollBar = HasScrollBar(TRUE);
}

void SImRichEdit::DoneUpdate(const UpdateContext& context)
{
	if (context.bHasScrollBar != HasScrollBar(TRUE))
	{
		//�����������˱仯,ȫ������Ҫ���²���
		for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
			_richContents.GetAt(npos)->OffsetCharRange(0);
	}

	if (context.bReadOnly)
		SetReadOnly(TRUE);
}

/*
 * ���������±���ʱ������ֱ�Ӹ��µ�DC�ϵģ������Ǵ���ײ��windowһֱ����richedit��һ�㣬
 * ���Ը�����SImRichEdit�ϵĿؼ����ᱻ�������ú�����SImRichEdit���ӿؼ�λ�ö���������
 *
 * @param pWnd: ���ڵ�ָ�룬ͨ����richedit��ָ��
 * @param prgn: ���ڵı���RGN
 *
 * �ú�������pWnd���Ӵ��ڣ����Ӵ��ڵ�λ�ô�RGN��ȥ����
 */
void SImRichEdit::ClipChildren(SWindow* pWnd, IRegionS* prgn)
{
	if (pWnd == NULL)
		return;

	for (SWindow* p = pWnd->GetWindow(GSW_FIRSTCHILD); p != NULL; p = p->GetWindow(GSW_NEXTSIBLING))
	{
		if (p->IsVisible())
		{
			CRect rc = p->GetClientRect();
			prgn->CombineRect(rc, RGN_DIFF);
		}
	}
}

void SImRichEdit::DrawScrollbar(BOOL bVertical, UINT uCode, int nPos)
{
	SCROLLINFO* psi = bVertical ? (&m_siVer) : (&m_siHoz);

	if (uCode != SB_THUMBTRACK && IsVisible(TRUE))
	{
		CRect rcRail = GetScrollBarRect(bVertical);
		rcRail.DeflateRect(GetScrollBarArrowSize(bVertical), 0);

		CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(&rcRail, GRT_PAINTBKGND, FALSE);
		//m_pSkinSb->Draw(pRT, rcRail, MAKESBSTATE(SB_PAGEDOWN, SBST_NORMAL, bVertical));
		psi->nTrackPos = -1;
		CRect rcSlide = GetScrollBarRect(bVertical);
		//m_pSkinSb->Draw(pRT, rcSlide, MAKESBSTATE(SB_THUMBTRACK, SBST_NORMAL, bVertical));
		ReleaseRenderTarget(pRT);
	}
}

void SImRichEdit::SetContentsDirty()
{
	for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
	{
		RichEditContent* pContent = _richContents.GetAt(npos);
		pContent->SetDirty(TRUE);
	}
}

void SImRichEdit::DrawVisibleGifs(IRenderTarget* pRt, const CRect& validRgnRect)
{
	CComPtr<IRichEditOle>  ole;
	SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);

	for (int i = _visibleOleChr.cpMin; i <= _visibleOleChr.cpMax; i++)
	{
		REOBJECT reobj = { 0 };
		reobj.cbStruct = sizeof(REOBJECT);

		HRESULT hr = ole->GetObject(i, &reobj, REO_GETOBJ_POLEOBJ);
		if (FAILED(hr))
		{
			break;
		}

// 		SComPtr<RichEditImageOle> pImageOle = NULL;
// 		if (reobj.poleobj->QueryInterface(IID_ImageOleCtrl, (VOID**)&pImageOle) == S_OK)
// 		{
// 			CRect rcObj = pImageOle->GetRect();
// 			if (rcObj.IntersectRect(rcObj, validRgnRect))
// 			{
// 				pImageOle->InternalDraw(pRt, _pDelayDrawRgn, reobj.cp);
// 			}
// 			reobj.poleobj->Release();
// 		}
	}
}

void SImRichEdit::DirectDrawOles(IRegionS* prgn)
{
	if ((GetState() & WndState_Invisible) ||  // ��������&&,������IsVisible���ж�
		!prgn ||
		prgn->IsEmpty() ||
		_isBackgroundDirty ||
		!_pBackgroundRt)
	{
		return;
	}

	CRect rcCli = GetClientRect();
	prgn->CombineRect(rcCli, RGN_AND);
	ClipChildren(this, prgn);

	CRect rcRegion;
	prgn->GetRgnBox(rcRegion);

	CAutoRefPtr<IRenderTarget> pRt = GetRenderTarget(&rcRegion, GRT_PAINTBKGND, 0);
	pRt->PushClipRegion(prgn);
	pRt->BitBlt(rcRegion, _pBackgroundRt, rcRegion.left, rcRegion.top);

	DrawVisibleContents(pRt);
	DrawVisibleGifs(pRt, rcRegion);

	ReleaseRenderTarget(pRt);
	pRt->PopClip();
}

void SImRichEdit::DrawVisibleContents(IRenderTarget* pRt)
{
	for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
	{
		RichEditContent* pContent = _richContents.GetAt(npos);
		CHARRANGE chr = pContent->GetCharRange();

		if (chr.cpMax < _visibleChr.cpMin || chr.cpMin > _visibleChr.cpMax)
			continue;

		pContent->DrawObject(pRt);
	}
}

void SImRichEdit::UpdateVisibleCharRanges()
{
	//
	// ���¿ɼ��ַ���Χ
	//
	CRect clientRect = GetClientRect();
	int nFirstVisibleLine = GetFirstVisibleLine();

	_visibleChr.cpMin = LineIndex(nFirstVisibleLine);
	_visibleChr.cpMax = CharFromPos(clientRect.BottomRight());

	//
	// ���¿ɼ�ole��Χ
	//

	CComPtr<IRichEditOle>  ole;
	SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);

	int oleCount = ole->GetObjectCount();
	_visibleOleChr.cpMin = FindFirstOleInrange(ole, 0, oleCount, _visibleChr.cpMin, _visibleChr.cpMax);
	_visibleOleChr.cpMax = (_visibleOleChr.cpMin == -1)
		? -1
		: FindLastOleInrange(ole, _visibleOleChr.cpMin, oleCount, _visibleChr.cpMin, _visibleChr.cpMax);
}

BOOL SImRichEdit::RecalcScrollbarPos(BOOL bVertical, UINT uCode, int nPos)
{
	SCROLLINFO* psi = bVertical ? (&m_siVer) : (&m_siHoz);
	int nNewPos = psi->nPos;
	int maxScrollPos = psi->nMax - psi->nPage;

	switch (uCode)
	{
	case SB_LINEUP:
		nNewPos -= GetScrollLineSize(bVertical);
		break;
	case SB_LINEDOWN:
		nNewPos += GetScrollLineSize(bVertical);
		break;
	case SB_PAGEUP:
		nNewPos -= psi->nPage;
		break;
	case SB_PAGEDOWN:
		nNewPos += psi->nPage;
		break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		nNewPos = nPos;
		break;
	case SB_TOP:
		nNewPos = psi->nMin;
		break;
	case SB_BOTTOM:
		nNewPos = maxScrollPos;
		break;
	}

	if (nNewPos < psi->nMin)
	{
		nNewPos = psi->nMin;
	}

	if (nNewPos > maxScrollPos)
	{
		nNewPos = maxScrollPos;
	}

	if (psi->nPage == 0)
	{
		nNewPos = 0;
	}

	BOOL scrollposChanged = FALSE;
	if (nNewPos != m_siVer.nPos)
	{
		psi->nPos = nNewPos;
		scrollposChanged = TRUE;
	}

	if (abs(nNewPos - maxScrollPos) <= 3)
	{
		_scrollbarAtBottom = TRUE;
	}
	else
	{
		_scrollbarAtBottom = FALSE;
	}

	return scrollposChanged;
}

HRESULT SImRichEdit::DefAttributeProc(const SStringW& strAttribName, const SStringW& strValue, BOOL bLoading)
{
	m_dwStyle |= ES_NOHIDESEL;

	HRESULT hr = SRichEdit::DefAttributeProc(strAttribName, strValue, bLoading);

	if (strAttribName.CompareNoCase(L"readOnly") == 0 ||
		strAttribName.CompareNoCase(L"enableDragdrop") == 0)
	{
		/*
		 * richedti����קЧ�������Ƿ�readonly����
		 * �����Ļ���Ҫ��iRichEditOleCallback�����ֽţ��ж����richedit��readonly���Ͳ��ܼ���ճ��
		 */
		if (!bLoading)
			EnableDragDrop(m_fEnableDragDrop);
	}

	return hr;
}

void SImRichEdit::OnNextFrame()
{
	time_t nTicksNow = GetTickCount();
	time_t nInterval = nTicksNow - _lastDrawTicks;

	if (!_pDelayDrawRgn)
	{
		return;
	}

	if (nInterval > 60 && !_pDelayDrawRgn->IsEmpty()) // 60 ��ˢ��һ��
	{
		_lastDrawTicks = nTicksNow;
		DirectDrawOles(_pDelayDrawRgn);
		_pDelayDrawRgn->Clear();
	}
}

LRESULT SImRichEdit::OnCreate(LPVOID)
{
	if (0 != __super::OnCreate(NULL))
		return 1;

	EnableDragDrop(m_fEnableDragDrop);
	_readOnlyBeforeDrag = GetReadOnly();

	GUID guid = __uuidof(ITextDocument);
	CComPtr<IRichEditOle> pOle = NULL;
	SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&pOle);
	HRESULT hr = pOle->QueryInterface(guid, (void**)&_pTextDoc);
	SASSERT(SUCCEEDED(hr));

	//
	// set IME
	//
	//DWORD dw = SSendMessage(EM_GETEDITSTYLE);
	//dw |= SES_USECTF;
	//SSendMessage(EM_SETEDITSTYLE, dw, dw);

	RichEditOleCallback* pcb = new RichEditOleCallback(this);
	SSendMessage(EM_SETOLECALLBACK, 0, (LPARAM)pcb);
	pcb->Release();
	GetContainer()->RegisterTimelineHandler(this);

	return 0;
}

LRESULT SImRichEdit::OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
{
#define DTOHIMETRIC(d, dpi) (LONG)MulDiv(d, HIMETRIC_PER_INCH, dpi)
	if (_originalInset.IsRectNull())
		_originalInset = m_rcInset;

	m_rcInset = _originalInset;

	if (HasScrollBar(TRUE))
		m_rcInset.right -= GetScrollBarRect(TRUE).Width();

	if (HasScrollBar(FALSE))
		m_rcInset.bottom -= GetScrollBarRect(FALSE).Height();

	//
	// copy from SPanel
	// ������� TXTBIT_EXTENTCHANGE ��֪ͨ�ŵ�OnSize�ٷ���richedit�����������ճ�SRichEdit�Ĵ���
	// �ڷ���TXTBIT_EXTENTCHANGE��Ϣ��richeditʱ�����ܻᵼ�¸ú������롣
	// TXTBIT_EXTENTCHANGE -> SPanel::ShowScrollBar -> OnNcCalcSize
	// Ϊ���߼��������TXTBIT_EXTENTCHANGE��Ϣ�ŵ�OnSizeȥ����
	//
	SPanel::OnNcCalcSize(bCalcValidRects, lParam);
	CRect rcInsetPixel = m_rcInset;
	if (!m_fRich && m_fSingleLineVCenter && !(m_dwStyle & ES_MULTILINE))
		rcInsetPixel.top = rcInsetPixel.bottom = (m_rcClient.Height() - m_nFontHeight) / 2;

	m_siHoz.nPage = m_rcClient.Width() - rcInsetPixel.left - rcInsetPixel.right;
	m_siVer.nPage = m_rcClient.Height() - rcInsetPixel.top - rcInsetPixel.bottom;

	HDC hdc = GetDC(GetContainer()->GetHostHwnd());
	LONG xPerInch = ::GetDeviceCaps(hdc, LOGPIXELSX);
	LONG yPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(GetContainer()->GetHostHwnd(), hdc);

	m_sizelExtent.cx = DTOHIMETRIC(m_rcClient.Width(), xPerInch);
	m_sizelExtent.cy = DTOHIMETRIC(m_rcClient.Height(), yPerInch);

	m_rcInset.left = DTOHIMETRIC(m_rcInset.left, xPerInch);
	m_rcInset.right = DTOHIMETRIC(m_rcInset.right, xPerInch);
	m_rcInset.top = DTOHIMETRIC(m_rcInset.top, yPerInch);
	m_rcInset.bottom = DTOHIMETRIC(m_rcInset.bottom, yPerInch);

	return 0;
}

void SImRichEdit::OnSize(UINT nType, CSize size)
{
	__super::OnSize(nType, size);

	BOOL scrollAtBottom = _scrollbarAtBottom;
	int scrollPos = GetScrollPos(TRUE);

	UpdateBkgndRenderTarget();
	GetTextServices()->OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE, TXTBIT_EXTENTCHANGE);
	UpdateContentPosition(0);
	ForceUpdateLayout();

	/*
	 * ����������autoVScroll���������ᾭ���Ҷ���������Ҫ��̬����һ�ѡ�
	 * - ������������ڴ��ڵײ������ܴ��ڳߴ������ı䣬����Ҫ��֤������ʼ�����Ŵ��ڵײ�
	 * - ������������ڵײ�����Ҫ��֤�����������ڵ�ǰλ�á�
	 */
	if (scrollAtBottom)
		OnScroll(TRUE, SB_BOTTOM, 0);
	else if (GetScrollPos(TRUE) != scrollPos)
		OnScroll(TRUE, SB_THUMBPOSITION, scrollPos);
}

void SImRichEdit::OnDestroy()
{
	if (_pTextDoc)
		_pTextDoc->Release();

	Clear();

	SRichEdit::OnDestroy();
}

void SImRichEdit::OnPaint(IRenderTarget* pRT)
{
	if (_isBackgroundDirty && _pBackgroundRt)
	{
		CRect rcClient = GetClientRect();
		_pBackgroundRt->BitBlt(rcClient, pRT, rcClient.left, rcClient.top);

		if (!IsRenderTargetEmpty(_pBackgroundRt))
			_isBackgroundDirty = FALSE;
	}
	// �����¼�������ᵼ��content���ࣺ
	// * OnSize֮��
	// * OnScroll֮��
	// * ͼƬֻ��ʾһ���ʱ�򣬵��ͼƬ��ͼƬ�������ĳ���richedit��
	SetContentsDirty();

	if (!_isDrawEnable)
		return;

	/*
	 * �ⲿ�ֵĸĶ���Ҫ��Ҫȷ������richedit�Զ��޸Ĺ�����λ�ã��ο���wince richedit��Դ�롣
	 *
	 * ����������autoVScroll����OnPaintʱ���Զ����������λ�ã�������Ҫ���ֶ���ES_AUTOVSCROLL��ʽȥ��
	 * TxDraw()-> [richedit�ڲ���ջ] -> TxGetScrollBars()
	 * ȷ��richedit��TxDraw�ڼ䲻�����ù�����λ��
	 *
	 * ����ĳЩԭ�������Լ�ά����λ�ú�richeditά���Ĺ�����λ�ò���һ�£������������Լ���Ϊ׼��
	 */
	DWORD autoVScroll = m_dwStyle & ES_AUTOVSCROLL;
	m_dwStyle &= ~ES_AUTOVSCROLL;
	if (_scrollbarOutdated && _fixVScroll)
		OnScroll(TRUE, SB_THUMBPOSITION, m_siVer.nPos);

	DrawVisibleContents(pRT);
	SRichEdit::OnPaint(pRT);
	m_dwStyle |= autoVScroll;
}

void SImRichEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
	BOOL bHandled = FALSE;
	POINT pt = { point.x, point.y };
	RichEditObj* pRichObj = HitTest(pt);
	if (pRichObj != NULL && pRichObj->NeedToProcessMessage())
	{
		LPARAM lParam = MAKELPARAM(pt.x, pt.y);
		pRichObj->ProcessMessage(WM_LBUTTONDOWN, 0, lParam, bHandled);
		if (bHandled)
		{
			SetFocus();
		}
	}
	
	// 如果HitTest返回NULL或对象不需要处理消息，调用基类处理以获取焦点和正确的消息处理
	if (!bHandled)
	{
		__super::OnLButtonDown(nFlags, point);
	}

	SetMsgHandled(bHandled);
}

void SImRichEdit::OnLButtonDblClick(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);

	BOOL bHandled = FALSE;
	RichEditObj* pRichObj = HitTest(point);

	if (pRichObj != NULL && pRichObj->NeedToProcessMessage())
	{
		pRichObj->ProcessMessage(
			WM_LBUTTONDBLCLK, GetCurMsg()->wParam, GetCurMsg()->lParam, bHandled);

		SetMsgHandled(bHandled);
	}
}

void SImRichEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
	BOOL bHandled = FALSE;
	POINT pt = { point.x, point.y };
	RichEditObj* pRichObj = HitTest(pt);
	if (pRichObj != NULL && pRichObj->NeedToProcessMessage())
	{
		LPARAM lParam = MAKELPARAM(pt.x, pt.y);
		pRichObj->ProcessMessage(WM_LBUTTONUP, 0, lParam, bHandled);
	}

	SetMsgHandled(bHandled);
}

void SImRichEdit::OnRButtonDown(UINT nFlags, CPoint point)
{
	LRESULT result = 0;
	//SwndProc(GetCurMsg()->uMsg, GetCurMsg()->wParam, GetCurMsg()->lParam, result);
	SetFocus();
}

void SImRichEdit::OnRButtonUp(UINT nFlags, CPoint point)
{
	LRESULT result = 0;
	SwndProc(GetCurMsg()->uMsg, GetCurMsg()->wParam, GetCurMsg()->lParam, &result);
	FireCtxMenu(point);
}

void SImRichEdit::OnMouseMove(UINT nFlags, CPoint point)
{
	BOOL bHandled = FALSE;
	RichEditObj* pRichObj = HitTest(point);

	if (pRichObj != NULL && pRichObj->NeedToProcessMessage())
	{
		pRichObj->ProcessMessage(
			WM_MOUSEMOVE, GetCurMsg()->wParam, GetCurMsg()->lParam, bHandled);
	}

	if (_pLastHoverObj != NULL && pRichObj != _pLastHoverObj)
	{
		_pLastHoverObj->ProcessMessage(
			WM_MOUSELEAVE, GetCurMsg()->wParam, GetCurMsg()->lParam, bHandled);
	}
	_pLastHoverObj = pRichObj;

	_isDragging = TRUE;
	_readOnlyBeforeDrag = GetReadOnly();

	__super::OnMouseMove(nFlags, point);

	_isDragging = FALSE;
	if (_readOnlyBeforeDrag != GetReadOnly())
		SetReadOnly(_readOnlyBeforeDrag);
}

BOOL SImRichEdit::OnSetCursor(const CPoint& point)
{
	LPARAM lParam = 0;
	POINT pt = { point.x, point.y };
	RichEditObj* pRichObj = HitTest(pt);
	if (pRichObj != NULL && pRichObj->NeedToProcessMessage())
	{
		BOOL bHandled = FALSE;
		LPARAM lParam = MAKELPARAM(pt.x, pt.y);
		pRichObj->ProcessMessage(WM_SETCURSOR, 0, lParam, bHandled);

		return TRUE;
	}

	if (!m_style.m_strCursor.IsEmpty())
	{
		return SWindow::OnSetCursor(point);
	}

	return SRichEdit::OnSetCursor(point);
}

BOOL SImRichEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int nPos = GetScrollPos(TRUE);

	zDelta /= WHEEL_DELTA;
	int nFinalPos = nPos + zDelta * m_nScrollSpeed * -1;
	if (nFinalPos < 0)
		nFinalPos = 0;

	if (nFinalPos == 0)
	{
// 		EventRichEditScroll evt(this);
// 		evt.WheelDelta = zDelta;
// 		evt.ScrollAtTop = TRUE;
// 
// 		FireEvent(evt);
	}

	INT nmin, nmax;
	GetScrollRange(TRUE, &nmin, &nmax);
	if (nFinalPos == nPos)
		return FALSE;

	OnScroll(TRUE, SB_THUMBTRACK, nFinalPos);
	DrawScrollbar(TRUE, SB_THUMBPOSITION, nFinalPos);

	return FALSE;
}

BOOL SImRichEdit::OnUpdateToolTip(CPoint pt, SwndToolTipInfo& tipInfo)
{
	POINT point = { pt.x, pt.y };
	RichEditObj* pContent = HitTest(pt);

	if (pContent != NULL)
	{
		CRect rcOle = pContent->GetRect();
		point.x = point.x - rcOle.left;
		point.y = point.y - rcOle.top;

		if (pContent->OnUpdateToolTip(point, tipInfo))
		{
			tipInfo.rcTarget.OffsetRect(rcOle.TopLeft());
			return TRUE;
		}
	}

	return __super::UpdateToolTip(pt, tipInfo);
}

//------------------------------------------------------------------------------
//
// event handlers
// ��Ϊ�û����鷽���ԭ�򣬿�����richedit��autoVScroll�����ǿ���������ܺ�richedit�ںܶ�
// �ڲ��Ĵ��������������ù�������λ�ã����¹����������Ҷ����������Ŵ��ڳߴ磬��קѡ�����ݵȡ�
// 
// Ϊ����richedit�Ĺ��������ܿ��ƣ���������Ϣ�Ĵ����������������˴�����
//  - OnSize
//  - OnPaint
//  - OnScroll
//  - OnNcCalcSize
// ������˼�������֪��richedit��ı�������ĵط��ȼ�¼ԭ����������λ�ã���richedit����
// ��������λ�ú����������û�ȥ��
//
// ��������ᵼ��richeditλ�ò���
//  - �ڹ������쵽�ײ�ʱ������������ײ��Ŀհף�������richedit����һ��SB_PAGEDOWN
//  - �ڹ������쵽�ײ�����autoVScroll����ʱ��ѡ��һ�������ݣ��������������Զ����¹���
// 
//------------------------------------------------------------------------------

/*
 * #define SB_LINEUP           0
 * #define SB_LINEDOWN         1
 * #define SB_PAGEUP           2
 * #define SB_PAGEDOWN         3
 * #define SB_THUMBPOSITION    4 (Up/Down,Drag,EM_EXLINEFROMCHAR,TXTBIT_EXTENTCHANGE,TXTBIT_SCROLLBARCHANGE)
 * #define SB_THUMBTRACK       5 (user drag thumb)
 * #define SB_TOP              6
 * #define SB_BOTTOM           7
 * #define SB_ENDSCROLL        8
 *
 * ע�⣺
 *  - ����Ҫ��������SB_THUMBPOSITION�������ػ棬��Ϊͨ��������richedit�Լ�����ģ����richedit��ͨ���ص������ػ档
 */
BOOL SImRichEdit::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
	RecalcScrollbarPos(bVertical, uCode, nPos);

	//if (m_strName != _T("EdtInput"))
	//{
	//    CRect rc = GetClientRect();
	//    STRACE(_T("onscroll code:%d, min:%d, max:%d, pos:%d, height:%d, realPos:%d"),
	//        uCode, m_siVer.nMin, m_siVer.nMax, nPos, rc.Height(), m_siVer.nPos);
	//}

	if (!m_fScrollPending)
	{
		_scrollbarOutdated = FALSE;

		UINT code = (uCode == SB_THUMBTRACK) ? SB_THUMBPOSITION : uCode;

		GetTextServices()->TxSendMessage(
			bVertical ? WM_VSCROLL : WM_HSCROLL,
			MAKEWPARAM(code, m_siVer.nPos),
			0,
			NULL);
	}
	else
	{
		/*
		 * �û���ק���ݵ��¹����������ı�ʱ����������richedit�ڲ����������⣬���¹�����λ�ó�����
		 *
		 * ���� m_fScrollPending ΪTRUE��˵������richedit��������TxSetScroll�����OnScroll��
		 * Ϊ������ѭ�����������������TxSendMessage���ù�����λ�á�
		 *
		 * ���������ǹ�����λ����ʧЧ�����Լ�ά���Ĺ�����λ��Ϊ׼����OnPaintʱ�������ù�����λ��
		 */
		LONG pos = 0;
		//m_pTxtHost->GetTextService()->TxGetVScroll(NULL, NULL, &pos, NULL, NULL);

		if (pos != m_siVer.nPos)
		{
			_scrollbarOutdated = TRUE;
		}

		UpdateVisibleCharRanges();

		if (_scrollbarAtBottom)
		{
			// notify
// 			EventRichEditScroll evt(this);
// 			evt.ScrollAtBottom = TRUE;
// 			FireEvent(evt);
		}
	}

	if (_pDelayDrawRgn)
		_pDelayDrawRgn->Clear();

	if (uCode == SB_THUMBTRACK || uCode == SB_LINEDOWN || uCode == SB_LINEUP ||
		uCode == SB_PAGEUP || uCode == SB_PAGEDOWN)
	{
		DirectDraw(m_rcClient);
	}

	return 0;
}

LRESULT	SImRichEdit::OnImeStartComposition(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_isCreatIme = true;
	SetMsgHandled(FALSE);	//��������Ϣ���´�
	return S_OK;
}

LRESULT	SImRichEdit::OnImeComposition(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_onPostImmComposition(uMsg, wParam, lParam);
	SetMsgHandled(FALSE);
	return 0;
}

BOOL SImRichEdit::_onPostImmComposition(UINT msg, WPARAM wParam, LPARAM lParam)
{
	HIMC hIMC = NULL;

	// 检查容器是否存在
	ISwndContainer* pContainer = GetContainer();
	if (!pContainer)
		return FALSE;

	if (hIMC = ImmGetContext(pContainer->GetHostHwnd()))
	{
		POINT pt = { 0, 0 };
		GetCaretPos(&pt);
		COMPOSITIONFORM CompForm = { 0 };
		ImmGetCompositionWindow(hIMC, &CompForm);

		LONG x;
		if (_isCreatIme)
			x = pt.x;
		else
			x = CompForm.ptCurrentPos.x;

		_isCreatIme = false;

		HDC hDC = GetDC(NULL);
		if (!hDC)
		{
			ImmReleaseContext(pContainer->GetHostHwnd(), hIMC);
			return FALSE;
		}

		LONG yPixPerInch = GetDeviceCaps(hDC, LOGPIXELSY);
		ReleaseDC(NULL, hDC);

		CompForm.dwStyle = CFS_FORCE_POSITION;
		CompForm.ptCurrentPos.x = x;
		int offset = m_cfDef.yHeight * yPixPerInch / LY_PER_INCH;
		CRect careRect = { 0, 0, 0, 0 };
		GetCaretRect2(careRect);
		CompForm.ptCurrentPos.y = pt.y + careRect.Height() - offset;
		ImmSetCompositionWindow(hIMC, &CompForm);

		CANDIDATEFORM candidateForm = { 0 };
		candidateForm.dwIndex = 0;
		candidateForm.dwStyle = CFS_CANDIDATEPOS;
		candidateForm.ptCurrentPos.x = x;
		candidateForm.ptCurrentPos.y = pt.y + careRect.Height() - offset + 5;

		ImmSetCandidateWindow(hIMC, &candidateForm);

		ImmReleaseContext(pContainer->GetHostHwnd(), hIMC);
	}

	return TRUE;
}