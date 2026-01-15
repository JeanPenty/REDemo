#include "stdafx.h"
#include "RichEditOleBase.h"

namespace SOUI
{
	// OleWindow
	BOOL OleWindow::m_bTiemrRegistered;
	OleTimerHandler OleWindow::m_timerHandler;

	OleWindow::OleWindow() :m_bDelayDraw(FALSE)
	{
		static int nIDGen = 0;
		m_nWindowID = nIDGen++;
	}

	OleWindow:: ~OleWindow()
	{

	}

	BOOL OleWindow::OnFireEvent(IEvtArgs* evt)
	{
		return FALSE;
	}

	CRect OleWindow::GetContainerRect()
	{
		return m_rcOleWindow;
	}

	HWND OleWindow::GetHostHwnd() SCONST
	{
		return m_pHostContainer->GetHostHwnd();
	}

	LPCWSTR OleWindow::GetTranslatorContext() SCONST
	{
		return m_pHostContainer->GetTranslatorContext();
	}

	void OleWindow::GetContainerRect(RECT* ret) SCONST
	{
		return m_pHostContainer->GetContainerRect(ret);
	}

	void OleWindow::UpdateRegion(IRegionS* rgn)
	{
		return m_pHostContainer->UpdateRegion(rgn);
	}

	void OleWindow::OnRedraw(LPCRECT rc, BOOL bClip)
	{
		if (m_rcOleWindow.IsRectNull())
			return;

		if (m_bDelayDraw)
			m_pHostRichEdit->DelayDraw(m_rcOleWindow);
		else
			m_pHostRichEdit->DirectDraw(m_rcOleWindow);
	}

	BOOL OleWindow::UpdateWindow(BOOL bForce/* = TRUE*/)
	{
		return m_pHostContainer->UpdateWindow(bForce);
	}

	BOOL OleWindow::UpdateWindow(void)
	{
		return m_pHostContainer->UpdateWindow();
	}

	void OleWindow::SetToolTip(LPCRECT rc, UINT tipAlign, LPCTSTR pszTip)
	{
		//
	}

	void OleWindow::EnableIME(BOOL bEnable)
	{
		//
	}

	void OleWindow::OnUpdateCursor()
	{
		//
	}

	void OleWindow::EnableHostPrivateUiDef(BOOL bEnable)
	{
		//
	}

	BOOL OleWindow::PostTask(IRunnable* runable, BOOL bAsync/* = TRUE*/)
	{
		return m_pHostContainer->PostTask(runable, bAsync);
	}

	int OleWindow::RemoveTasksForObject(void* pObj)
	{
		return m_pHostContainer->RemoveTasksForObject(pObj);
	}

// 	IRenderTarget* OleWindow::OnGetRenderTarget(const CRect& rc, DWORD gdcFlags)
// 	{
// 		return m_pHostContainer->OnGetRenderTarget(rc, gdcFlags);
// 	}

// 	void OleWindow::OnReleaseRenderTarget(IRenderTarget* pRT, const CRect& rc, DWORD gdcFlags)
// 	{
// 		m_pHostContainer->OnReleaseRenderTarget(pRT, rc, gdcFlags);
// 	}

	void OleWindow::RealDraw(CRect rc)
	{
		m_pHostRichEdit->DirectDraw(rc);
	}

	BOOL OleWindow::IsTranslucent() const
	{
		return TRUE;
	}

	BOOL OleWindow::IsSendWheel2Hover() const
	{
		return TRUE;
	}

	BOOL OleWindow::RegisterTimelineHandler(ITimelineHandler* pHandler)
	{
		if (!m_bTiemrRegistered)
		{
			m_pHostContainer->RegisterTimelineHandler(&m_timerHandler);
			m_bTiemrRegistered = TRUE;
		}

		//return m_pHostContainer->RegisterTimelineHandler(pHandler);
		return m_timerHandler.RegisterHandler(pHandler);
	}

	BOOL OleWindow::UnregisterTimelineHandler(ITimelineHandler* pHandler)
	{
		//return m_pHostContainer->UnregisterTimelineHandler(pHandler);
		m_bTiemrRegistered = FALSE;
		return m_timerHandler.UnregisterHandler(pHandler);
	}

	IMessageLoop* OleWindow::GetMsgLoop()
	{
		return m_pHostContainer->GetMsgLoop();
	}

	LRESULT OleWindow::HandleEvent(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return DoFrameEvent(msg, wParam, lParam);
	}

	void OleWindow::SetHostRichEdit(IRichEditObjHost* pRichEdit)
	{
		m_pHostRichEdit = pRichEdit;
		m_pHostContainer = pRichEdit->GetHostContainer();
	}


	////////////////////////////////////////////////////////////////////////////////////////
	//RichEditOleAdapter
	RichEditOleAdapter::RichEditOleAdapter(RichEditOleBase* pHost)
		: _pHost(pHost), _comReferences(1)
	{
		SASSERT(pHost != NULL);
	}

	RichEditOleAdapter::~RichEditOleAdapter()
	{
		_pHost = NULL;
	}

	// IUnknown methods
	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::QueryInterface(REFIID riid, void** ppvObject)
	{
		if (ppvObject == NULL)
			return E_POINTER;

		if (::IsEqualIID(riid, IID_IUnknown))
		{
			*ppvObject = static_cast<IUnknown*>(static_cast<IOleObject*>(this));
		}
		else if (::IsEqualIID(riid, IID_IOleObject))
		{
			*ppvObject = static_cast<IOleObject*>(this);
		}
		else if (::IsEqualIID(riid, IID_IViewObject))
		{
			*ppvObject = static_cast<IViewObject*>(this);
		}
		else if (::IsEqualIID(riid, IID_IViewObject2))
		{
			*ppvObject = static_cast<IViewObject2*>(this);
		}
		else
		{
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}

	ULONG STDMETHODCALLTYPE RichEditOleAdapter::AddRef(void)
	{
		return ++_comReferences;
	}

	ULONG STDMETHODCALLTYPE RichEditOleAdapter::Release(void)
	{
		ULONG nRef = --_comReferences;
		if (nRef == 0)
		{
			delete this;
		}
		return nRef;
	}

	// IOleObject methods - 大部分返回 E_NOTIMPL 或 S_OK
	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::SetHostNames(LPCOLESTR, LPCOLESTR)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::Close(DWORD dwSaveOption)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::SetMoniker(DWORD dwWhich, IMoniker* pmk)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::GetMoniker(DWORD, DWORD, IMoniker** ppmk)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::InitFromData(IDataObject*, BOOL, DWORD)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::GetClipboardData(DWORD, IDataObject**)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::DoVerb(LONG iVerb, LPMSG lpmsg, IOleClientSite* pActiveSite,
		LONG lindex, HWND hwndParent, LPCRECT lprcPosRect)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::EnumVerbs(IEnumOLEVERB** ppEnumOleVerb)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::Update(void)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::IsUpToDate(void)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::GetUserType(DWORD, LPOLESTR* pszUserType)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::EnumAdvise(IEnumSTATDATA** ppenumAdvise)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::GetMiscStatus(DWORD, DWORD* pdwStatus)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::SetColorScheme(LOGPALETTE* pLogpal)
	{
		return E_NOTIMPL;
	}

	// 需要真正实现的方法 - 转发到 RichEditOleBase
	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::GetUserClassID(CLSID* pClsid)
	{
		if (_pHost == NULL)
			return E_FAIL;
		return _pHost->GetUserClassID(pClsid);
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::Advise(IAdviseSink* pAdvSink, DWORD* pdwConnection)
	{
		if (_pHost == NULL)
			return E_FAIL;
		return _pHost->Advise(pAdvSink, pdwConnection);
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::Unadvise(DWORD dwConnection)
	{
		if (_pHost == NULL)
			return E_FAIL;
		return _pHost->Unadvise(dwConnection);
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::SetClientSite(IOleClientSite* pClientSite)
	{
		if (_pHost == NULL)
			return E_FAIL;
		return _pHost->SetClientSite(pClientSite);
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::GetClientSite(IOleClientSite** ppClientSite)
	{
		if (_pHost == NULL)
			return E_FAIL;
		return _pHost->GetClientSite(ppClientSite);
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::GetExtent(DWORD dwDrawAspect, SIZEL* psizel)
	{
		if (_pHost == NULL)
			return E_FAIL;
		return _pHost->GetExtent(dwDrawAspect, psizel);
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::SetExtent(DWORD dwDrawAspect, SIZEL* psizel)
	{
		if (_pHost == NULL)
			return E_FAIL;
		return _pHost->SetExtent(dwDrawAspect, psizel);
	}

	// IViewObject2 methods
	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::GetColorSet(DWORD, LONG, void*, DVTARGETDEVICE*, HDC, LOGPALETTE**)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::Freeze(DWORD, LONG, void*, DWORD*)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::Unfreeze(DWORD dwFreeze)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::SetAdvise(DWORD aspects, DWORD advf, IAdviseSink* pAdvSink)
	{
		if (_pHost == NULL)
			return E_FAIL;
		return _pHost->SetAdvise(aspects, advf, pAdvSink);
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::GetAdvise(DWORD* pAspects, DWORD* pAdvf, IAdviseSink** ppAdvSink)
	{
		if (_pHost == NULL)
			return E_FAIL;
		return _pHost->GetAdvise(pAspects, pAdvf, ppAdvSink);
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::Draw(DWORD dwDrawAspect, LONG lindex, void* pvAspect,
		DVTARGETDEVICE* ptd, HDC hdcTargetDev, HDC hdcDraw,
		LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
		BOOL(STDMETHODCALLTYPE* pfnContinue)(ULONG_PTR dwContinue),
		ULONG_PTR dwContinue)
	{
		if (_pHost == NULL)
			return E_FAIL;
		return _pHost->Draw(dwDrawAspect, lindex, pvAspect, ptd, hdcTargetDev, hdcDraw,
			lprcBounds, lprcWBounds, pfnContinue, dwContinue);
	}

	HRESULT STDMETHODCALLTYPE RichEditOleAdapter::GetExtent(DWORD dwDrawAspect, LONG lindex,
		DVTARGETDEVICE* ptd, LPSIZEL lpsizel)
	{
		if (_pHost == NULL)
			return E_FAIL;
		return _pHost->GetExtent(dwDrawAspect, lindex, ptd, lpsizel);
	}




	///////////////////////////////////
	//RichEditOleBase

	RichEditOleBase::RichEditOleBase() :_canBeSelect(TRUE), _pComAdapter(NULL)
	{
		_sizeNatural.cx = _sizeNatural.cy = 0;
		_sizeExtent.cx = _sizeExtent.cy = 0;

		// 创建 COM 适配器
		_pComAdapter = new RichEditOleAdapter(this);
	}

	RichEditOleBase::~RichEditOleBase()
	{
		//_oleView.SendMessage(WM_DESTROY);
		// 释放 COM 适配器
		if (_pComAdapter)
		{
			_pComAdapter->Release();
			_pComAdapter = NULL;
		}
	}

	// 实现 SOUI::IObject::OnFinalRelease，保持空实现以满足接口要求
	void RichEditOleBase::OnFinalRelease()
	{
		// nothing to do here; provided to satisfy abstract interface
	}

	// 获取 COM 接口适配器
	RichEditOleAdapter* RichEditOleBase::GetComAdapter()
	{
		return _pComAdapter;
	}

	HRESULT RichEditOleBase::QueryInterface(REFIID riid, void** ppvObject)
	{
		return _pComAdapter->QueryInterface(riid, ppvObject);
	}

	LONG RichEditOleBase::AddRef(void)
	{
		return (LONG)_pComAdapter->AddRef();
	}

	LONG RichEditOleBase::Release(void)
	{
		return (LONG)_pComAdapter->Release();
	}

	void RichEditOleBase::SetDirty(BOOL bDirty)
	{
		RichEditObj::SetDirty(bDirty);
		if (bDirty)
		{
			_objRect.SetRect(0, 0, 0, 0);
			_oleView.SetOleWindowRect(CRect(0, 0, 0, 0));
		}
		else
		{
			_oleView.SetOleWindowRect(_objRect);
		}
	}

	LRESULT RichEditOleBase::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		AdjustMessageParam(msg, wParam, lParam);
		_oleView.HandleEvent(msg, wParam, lParam);
		bHandled = _oleView.IsMsgHandled();

		//
		// bHandled在此时总是为TRUE。
		// 通常情况下，我们希望消息继续由RichEdit控件继续处理，所以这里直接设为FALSE。
		// 如果某些OLE不希望消息由RichEdit处理，需要在子类中重载处理。比如点击一个OLE，
		// 默认情况会出现一个黑框，如果这里的bHandled为TRUE，则RichEdit就没机会处理这个点击事件，也就不会出现黑框
		//
		bHandled = FALSE;
		if (!_canBeSelect && (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK))
		{
			bHandled = TRUE; // 文件OLE不让RichEdit继续左击，否则会画出一个黑框
		}
		//无效果
		if (!_canBeSelect && (msg == WM_SETFOCUS))
		{
			_oleView.SSendMessage(WM_KILLFOCUS);
		}
		return 0;
	}

	BOOL RichEditOleBase::InsertIntoHost(IRichEditObjHost* pHost)
	{
		SASSERT(pHost);
		_pObjHost = pHost;    // 要放在第一句,其它函数要用到m_pRichEditHost

		InitOleWindow(pHost);
		return SUCCEEDED(InsertOleObject(pHost));
	}

	BOOL RichEditOleBase::OnUpdateToolTip(CPoint pt, SwndToolTipInfo& tipInfo)
	{
		SWindow* pChild = _oleView.GetWindow(GSW_LASTCHILD);
		if (!pChild)
		{
			return FALSE;
		}

		SWND hHover = pChild->SwndFromPoint(pt, FALSE);
		SWindow* pHover = SWindowMgr::GetWindow(hHover);

		if (pHover)
		{
			if (pHover->UpdateToolTip(pt, tipInfo))
			{
				tipInfo.dwCookie = pHover->GetSwnd();
				return TRUE;
			}
		}
		return FALSE;
	}

	BOOL RichEditOleBase::InitOleWindow(IRichEditObjHost* pHost)
	{
		BOOL bRet = FALSE;
		if (!_xmlLayout.IsEmpty())
		{
			pugi::xml_document xmlDoc;
			SStringTList strLst;

			if (2 == ParseResID(_xmlLayout, strLst))
			{
				//load xml
				//LOADXML(xmlDoc, strLst[1], strLst[0]);
			}
			else
			{
				//load xml
				//LOADXML(xmlDoc, strLst[0], _T("LAYOUT"));
			}

			if (xmlDoc)
			{
				_oleView.SetHostRichEdit(pHost);
				SXmlNode node(xmlDoc.child(L"root"));
				bRet = _oleView.InitFromXml(&node);
				SASSERT(bRet);
				_oleView.Move2(0, 0, _sizeNatural.cx, _sizeNatural.cy);
				CalculateExtentSize(_sizeNatural);
			}
		}

		return bRet;
	}

	void RichEditOleBase::CalculateExtentSize(const SIZE& sizeNature)
	{
		// 周围留一个像素作为选中时的黑框
		HDC hDC = ::GetDC(NULL);
		_sizeExtent.cx = ::MulDiv(sizeNature.cx + 2, 2540, GetDeviceCaps(hDC, LOGPIXELSX));
		_sizeExtent.cy = ::MulDiv(sizeNature.cy + 2, 2540, GetDeviceCaps(hDC, LOGPIXELSY));
		::ReleaseDC(NULL, hDC);
	}

	void RichEditOleBase::RequestDraw()
	{
		SWindow* pRoot = _oleView.GetWindow(GSW_FIRSTCHILD);
		if (pRoot)
		{
			pRoot->Invalidate();
		}
	}

	void RichEditOleBase::UpdateWindowLayout(SWindow* pWindow)
	{
		if (!pWindow || !_pObjHost)
			return;

		_pObjHost->EnableDraw(FALSE);
		pWindow->RequestRelayout();
		pWindow->GetParent()->UpdateLayout();
		_pObjHost->EnableDraw(TRUE);
	}

	HRESULT RichEditOleBase::InsertOleObject(IRichEditObjHost* pHost)
	{
		//insert this to host
		SComPtr<IOleObject>	pOleObject;
		SComPtr<IOleClientSite> pClientSite;

		HRESULT hr = E_FAIL;
		REOBJECT reobject = { 0 };

		SComPtr<IRichEditOle> ole;
		pHost->SendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);

		// Get site
		ole->GetClientSite(&pClientSite);
		SASSERT(pClientSite != NULL);

		SComPtr<IRichEditOleCallback> pCallback;
		hr = ole->QueryInterface(IID_IRichEditOleCallback, (void**)&pCallback);
		if (!SUCCEEDED(hr)) return E_FAIL;

		//get the IOleObject
		hr = QueryInterface(IID_IOleObject, (void**)&pOleObject);
		if (FAILED(hr))
			return	 E_FAIL;

		//to insert into richedit, you need a struct of REOBJECT
		ZeroMemory(&reobject, sizeof(REOBJECT));

		pOleObject->GetUserClassID(&reobject.clsid);
		pCallback->GetNewStorage(&reobject.pstg);
		reobject.cbStruct = sizeof(REOBJECT);
		reobject.cp = REO_CP_SELECTION;
		reobject.dvaspect = DVASPECT_CONTENT;
		reobject.dwFlags = REO_BELOWBASELINE;
		reobject.poleobj = pOleObject;
		reobject.polesite = pClientSite;
		hr = pOleObject->SetClientSite(pClientSite);
		if (SUCCEEDED(hr))
		{
			PreInsertObject(reobject);  // 给子类一个机会去修改reobject
			hr = ole->InsertObject(&reobject);
			_contentChr.cpMin = pHost->GetCharCount() - 1;
			_contentChr.cpMax = _contentChr.cpMin + 1;
		}

		if (reobject.pstg)
			reobject.pstg->Release();

		return hr;
	}

	// 
	// 单选的时候才需要画反色框 
	//
	void RichEditOleBase::InvertBorder(HDC hdc, LPRECT lprcBorder)
	{
		if (!_canBeSelect)
		{
			CHARRANGE chr = { 0 };
			_pObjHost->SendMessage(EM_EXGETSEL, 0, (LPARAM)&chr, NULL);
			if (chr.cpMax - chr.cpMin == 1 &&       // |
				chr.cpMin <= _contentChr.cpMin &&  // -> 单选,并且选中了自己 
				_contentChr.cpMin < chr.cpMax)     // |
			{
				SComPtr<IRichEditOle> ole;
				_pObjHost->SendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);

				REOBJECT reobj = { 0 };
				reobj.cbStruct = sizeof(REOBJECT);
				reobj.cp = _contentChr.cpMin;
				HRESULT hr = ole->GetObject(REO_IOB_USE_CP, &reobj, REO_GETOBJ_NO_INTERFACES);
				if (SUCCEEDED(hr))
				{
//                     CRect rcBorder = lpBorder;
//                     InvertRect(hdc, rcBorder);
//                     rcBorder.InflateRect(-1, -1, -1, -1);
//                     InvertRect(hdc, rcBorder);
				}
			}
		}
	}

	HRESULT RichEditOleBase::GetUserClassID(CLSID* pClsid)
	{
		if (pClsid == NULL)
		{
			return E_POINTER;
		}
		*pClsid = _oleGuid;

		return S_OK;
	}

	HRESULT RichEditOleBase::Advise(IAdviseSink* pAdvSink, DWORD* pdwConnection)
	{
		_spAdviseSink = pAdvSink;
		return S_OK;
	}

	HRESULT RichEditOleBase::Unadvise(DWORD dwConnection)
	{
		HRESULT hRes = E_FAIL;
		if (_spOleAdviseHolder != NULL)
			hRes = _spOleAdviseHolder->Unadvise(dwConnection);
		return hRes;
	}

	HRESULT RichEditOleBase::SetClientSite(IOleClientSite* pClientSite)
	{
		_spClientSite = pClientSite;
		return S_OK;
	}

	HRESULT RichEditOleBase::GetClientSite(IOleClientSite** ppClientSite)
	{
		SASSERT(ppClientSite);
		if (ppClientSite == NULL)
			return E_POINTER;

		*ppClientSite = _spClientSite;
		if (_spClientSite != NULL)
			_spClientSite.p->AddRef();
		return S_OK;
	}

	HRESULT RichEditOleBase::GetExtent(DWORD dwDrawAspect, SIZEL* psizel)
	{
		if (dwDrawAspect != DVASPECT_CONTENT)
			return E_FAIL;

		if (psizel == NULL)
			return E_POINTER;

		*psizel = _sizeExtent;
		return S_OK;
	}

	HRESULT RichEditOleBase::SetExtent(DWORD dwDrawAspect, SIZEL* psizel)
	{
		if (dwDrawAspect != DVASPECT_CONTENT)
			return DV_E_DVASPECT;

		if (psizel == NULL)
			return E_POINTER;

		_sizeExtent = *psizel;

		return S_OK;
	}

	HRESULT RichEditOleBase::SetAdvise(DWORD aspects, DWORD advf, IAdviseSink* pAdvSink)
	{
		_spAdviseSink = pAdvSink;
		return S_OK;
	}

	HRESULT RichEditOleBase::GetAdvise(DWORD* pAspects, DWORD* pAdvf, IAdviseSink** ppAdvSink)
	{
		HRESULT hr = E_POINTER;
		if (ppAdvSink != NULL)
		{
			*ppAdvSink = _spAdviseSink;
			if (_spAdviseSink)
				_spAdviseSink.p->AddRef();
			hr = S_OK;
		}
		return hr;
	}

	HRESULT RichEditOleBase::Draw(
		DWORD dwDrawAspect, LONG lindex,
		void* pvAspect,
		DVTARGETDEVICE* ptd,
		HDC hdcTargetDev,
		HDC hdcDraw,
		LPCRECTL lprcBounds,
		LPCRECTL lprcWBounds,
		BOOL(*pfnContinue)(ULONG_PTR dwContinue),
		ULONG_PTR dwContinue)
	{
		InvertBorder(hdcDraw, (RECT*)lprcBounds);

		_objRect = (RECT*)lprcBounds;
		_objRect.InflateRect(-1, -1, -1, -1); // 四周留一个像素给RichEdit画反色框
		_oleView.SetOleWindowRect(_objRect);

		if (_objRect.Width() <= 0 || _objRect.Height() <= 0)
		{
			return S_OK;
		}

		CAutoRefPtr<IRegionS> rgn;
		GETRENDERFACTORY->CreateRegion(&rgn);
		rgn->CombineRect((RECT*)lprcBounds, RGN_AND);

		CAutoRefPtr<IRenderTarget> pRT;
		GETRENDERFACTORY->CreateRenderTarget(&pRT, _objRect.Width(), _objRect.Height());

		// 画背景
		HDC hdc = pRT->GetDC(0);
		::BitBlt(hdc, 0, 0, _objRect.Width(), _objRect.Height(),
			hdcDraw, _objRect.left, _objRect.top,
			SRCCOPY);
		pRT->ReleaseDC(hdc);

		// 画richedit
		_oleView.RedrawRegion(pRT, rgn);
		//_oleView.UpdateRegion(rgn);

		hdc = pRT->GetDC(0);
		// 贴到目标DC
		::BitBlt(hdcDraw, _objRect.left, _objRect.top, _objRect.Width(), _objRect.Height(),
			hdc, 0, 0,
			SRCCOPY);

		pRT->ReleaseDC(hdc);

		return S_OK;
	}

	HRESULT RichEditOleBase::GetExtent(
		DWORD dwDrawAspect,
		LONG lindex,
		DVTARGETDEVICE* ptd,
		LPSIZEL lpsizel)
	{
		SASSERT(lpsizel != NULL);
		if (lpsizel == NULL)
			return E_POINTER;

		*lpsizel = _sizeExtent;
		return S_OK;
	}
}