// ------------------------------------------------------------------------------
//
// RichEditOleBase.h : interface of the RichEditOleBase class
//
// 定义了RichEdit OLE控件的基类。OLE控件持有OleWindow对象，从而具有SWindow
// 的能力，如可使用XML文件定义外观，可以响应鼠标/键盘进行操作等。
//
// OLE控件主要由以下3个部分组成：
// - OleWindow      : 负责OLE控件的外观,交互
// - OleTimerHandler: OLE控件使用的定时器，为了大量GIF而优化的定时器
// - RichEditOleBase: 负责和RichEdit交互
//
// ------------------------------------------------------------------------------

#pragma once
#include "RichEditObj.h"
#include "core\SwndContainerImpl.h"
#include <atlcomcli.h>
#include <oleidl.h>
#include <objidl.h>
#include <richole.h>

namespace SOUI
{
	class OleTimerHandler : public ITimelineHandler
	{
	public:
		BOOL RegisterHandler(ITimelineHandler* pHandler)
		{
			SPOSITION pos = m_lstHandler.Find(pHandler);
			if (pos) return FALSE;
			m_lstHandler.AddTail(pHandler);
			return TRUE;
		}

		BOOL UnregisterHandler(ITimelineHandler* pHandler)
		{
			SPOSITION pos = m_lstHandler.Find(pHandler);
			if (!pos) return FALSE;
			m_lstHandler.RemoveAt(pos);
			return TRUE;
		}

		void OnNextFrame()
		{
			/*
			 * 如果有大量的定时器注册上来,每次都要拷贝,很费CPU,特意把以下两句去掉
			 * 注意在OnNextFrame里面不能注册和取消定时器,否则循环会出错
			 */
			 //SList<ITimelineHandler*> lstCopy;
			 //CopyList(m_lstHandler,lstCopy);

			SPOSITION pos = m_lstHandler.GetHeadPosition();
			while (pos)
			{
				ITimelineHandler* pHandler = m_lstHandler.GetNext(pos);
				pHandler->OnNextFrame();
			}
		}

	protected:
		SList<ITimelineHandler*>    m_lstHandler;
	};

	// OleWindow 
	class OleWindow : public SwndContainerImpl
	{
	public:
		OleWindow();
		virtual ~OleWindow();

		// ISwndContainer methods
		BOOL OnFireEvent(IEvtArgs* evt);
		CRect GetContainerRect();
		HWND GetHostHwnd() SCONST;
		LPCWSTR GetTranslatorContext() SCONST;
		void GetContainerRect(RECT* ret) SCONST;
		void UpdateRegion(IRegionS* rgn);
		void OnRedraw(LPCRECT rc, BOOL bClip);
		BOOL UpdateWindow(BOOL bForce = TRUE);
		BOOL UpdateWindow(void);
		void UpdateTooltip() {}
		void SetToolTip(LPCRECT rc, UINT tipAlign, LPCTSTR pszTip);
		void EnableIME(BOOL bEnable);
		void OnUpdateCursor();
		void EnableHostPrivateUiDef(BOOL bEnable);
		BOOL PostTask(IRunnable* runable, BOOL bAsync = TRUE);
		int RemoveTasksForObject(void* pObj);

// 		IRenderTarget* OnGetRenderTarget(const CRect& rc, DWORD gdcFlags);
// 		void OnReleaseRenderTarget(IRenderTarget* pRT, const CRect& rc, DWORD gdcFlags);
		
		BOOL IsTranslucent() const;
		BOOL IsSendWheel2Hover() const;
		BOOL OnCreateCaret(SWND swnd, HBITMAP hBmp, int nWidth, int nHeight) { return FALSE; }
		BOOL OnShowCaret(BOOL bShow) { return FALSE; }
		BOOL OnSetCaretPos(int x, int y) { return FALSE; }
		
		BOOL RegisterTimelineHandler(ITimelineHandler* pHandler);
		BOOL UnregisterTimelineHandler(ITimelineHandler* pHandler);
		IMessageLoop* GetMsgLoop();
		IScriptModule* GetScriptModule() { return NULL; }
		int GetScale() const { return 100; }

		// internal public methods
		void SetHostRichEdit(IRichEditObjHost*);
		LRESULT HandleEvent(UINT msg, WPARAM wParam, LPARAM lParam);
		void SetOleWindowRect(CRect rcWindow) { m_rcOleWindow = rcWindow; }
		void SetDelayDraw(BOOL bDelayDraw) { m_bDelayDraw = bDelayDraw; }
	protected:
		void RealDraw(CRect rc);
	private:
		ISwndContainer* m_pHostContainer;
		IRichEditObjHost* m_pHostRichEdit;
		int m_nWindowID;

		CRect m_rcOleWindow;      // ole在richedit里面的位置   
		BOOL m_bDelayDraw;       // 对于GIF，需要设置延迟刷新

		static OleTimerHandler m_timerHandler;     // 定时器
		static BOOL m_bTiemrRegistered;
	};

	class RichEditOleBase;

	class RichEditOleAdapter : public IOleObject
		, public IViewObject2
	{
	public:
		RichEditOleAdapter(RichEditOleBase* pHost);
		virtual ~RichEditOleAdapter();

		//
		// IUnknown meghods
		//
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
		ULONG   STDMETHODCALLTYPE AddRef(void);
		ULONG   STDMETHODCALLTYPE Release(void);

		//
		// IOleObject methods
		//
		HRESULT STDMETHODCALLTYPE SetHostNames(LPCOLESTR, LPCOLESTR);
		HRESULT STDMETHODCALLTYPE Close(DWORD dwSaveOption);
		HRESULT STDMETHODCALLTYPE SetMoniker(DWORD dwWhich, IMoniker* pmk);
		HRESULT STDMETHODCALLTYPE GetMoniker(DWORD, DWORD, IMoniker** ppmk);
		HRESULT STDMETHODCALLTYPE InitFromData(IDataObject*, BOOL, DWORD);
		HRESULT STDMETHODCALLTYPE GetClipboardData(DWORD, IDataObject**);
		HRESULT STDMETHODCALLTYPE DoVerb(LONG iVerb, LPMSG lpmsg, IOleClientSite* pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect);
		HRESULT STDMETHODCALLTYPE EnumVerbs(IEnumOLEVERB** ppEnumOleVerb);
		HRESULT STDMETHODCALLTYPE Update(void);
		HRESULT STDMETHODCALLTYPE IsUpToDate(void);
		HRESULT STDMETHODCALLTYPE GetUserType(DWORD, LPOLESTR* pszUserType);
		HRESULT STDMETHODCALLTYPE EnumAdvise(IEnumSTATDATA** ppenumAdvise);
		HRESULT STDMETHODCALLTYPE GetMiscStatus(DWORD, DWORD* pdwStatus);
		HRESULT STDMETHODCALLTYPE SetColorScheme(LOGPALETTE* pLogpal);

		//以下接口需要实现
		HRESULT STDMETHODCALLTYPE GetUserClassID(CLSID* pClsid);
		HRESULT STDMETHODCALLTYPE Advise(IAdviseSink* pAdvSink, DWORD* pdwConnection);
		HRESULT STDMETHODCALLTYPE Unadvise(DWORD dwConnection);
		HRESULT STDMETHODCALLTYPE SetClientSite(IOleClientSite* pClientSite);
		HRESULT STDMETHODCALLTYPE GetClientSite(IOleClientSite** ppClientSite);
		HRESULT STDMETHODCALLTYPE GetExtent(DWORD dwDrawAspect, SIZEL* psizel);
		HRESULT STDMETHODCALLTYPE SetExtent(DWORD dwDrawAspect, SIZEL* psizel);

		//
		// IViewObject2 methods
		//
		HRESULT STDMETHODCALLTYPE GetColorSet(DWORD, LONG, void*, DVTARGETDEVICE*, HDC, LOGPALETTE**);
		HRESULT STDMETHODCALLTYPE Freeze(DWORD, LONG, void*, DWORD*);
		HRESULT STDMETHODCALLTYPE Unfreeze(DWORD dwFreeze);

		// 以下接口需要实现
		HRESULT STDMETHODCALLTYPE SetAdvise(DWORD aspects, DWORD advf, IAdviseSink* pAdvSink);
		HRESULT STDMETHODCALLTYPE GetAdvise(DWORD* pAspects, DWORD* pAdvf, IAdviseSink** ppAdvSink);
		HRESULT STDMETHODCALLTYPE Draw(
			DWORD dwDrawAspect, LONG lindex,
			void* pvAspect,
			DVTARGETDEVICE* ptd,
			HDC hdcTargetDev,
			HDC hdcDraw,
			LPCRECTL lprcBounds,
			LPCRECTL lprcWBounds,
			BOOL(STDMETHODCALLTYPE* pfnContinue)(ULONG_PTR dwContinue),
			ULONG_PTR dwContinue);

		HRESULT STDMETHODCALLTYPE GetExtent(
			DWORD dwDrawAspect,
			LONG lindex,
			DVTARGETDEVICE* ptd,
			LPSIZEL lpsizel);

	private:
		RichEditOleBase* _pHost; // 宿主 RichEditOleBase 实例（非 owning）
		ULONG _comReferences;    // COM 接口的独立引用计数
	};


	class RichEditOleBase : public RichEditObj
	{
	public:
		RichEditOleBase();
		virtual ~RichEditOleBase();

		virtual void OnFinalRelease();
		// 获取 COM 接口适配器
		RichEditOleAdapter* GetComAdapter();

		HRESULT QueryInterface(REFIID riid, void** ppvObject);
		LONG   AddRef(void);
		LONG   Release(void);

		//
		// RichEditObj methods
		//
		void SetDirty(BOOL bDirty);
		LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		BOOL InsertIntoHost(IRichEditObjHost* pHost);
		BOOL OnUpdateToolTip(CPoint pt, SwndToolTipInfo& tipInfo);
		BOOL NeedToProcessMessage() { return TRUE; }

		//
		// RichEditOleBase public methods
		//
		virtual BOOL InitOleWindow(IRichEditObjHost* pHost);
		virtual void PreInsertObject(REOBJECT& object) {}
		void CalculateExtentSize(const SIZE& sizeNature);
		virtual SStringW GetSelFormatedText() { return L""; }
		virtual void RequestDraw();

	protected:
		void UpdateWindowLayout(SWindow* pWindow);
		HRESULT InsertOleObject(IRichEditObjHost*);
		void InvertBorder(HDC hdc, LPRECT lprcBorder);

		SOUI_ATTRS_BEGIN()
			ATTR_INT(L"selectable", _canBeSelect, FALSE)
			SOUI_ATTRS_END()

	protected:
		CComPtr<IOleClientSite>     _spClientSite;
		CComPtr<IOleAdviseHolder>   _spOleAdviseHolder;
		CComPtr<IAdviseSink>        _spAdviseSink;
		SIZEL                       _sizeExtent;
		OleWindow                   _oleView;
		SIZE                        _sizeNatural;
		GUID                        _oleGuid;
		SStringW                    _xmlLayout;
		BOOL                        _canBeSelect;

		RichEditOleAdapter* _pComAdapter;    // COM 接口适配器
	public:
		HRESULT GetUserClassID(CLSID* pClsid);
		HRESULT Advise(IAdviseSink* pAdvSink, DWORD* pdwConnection);
		HRESULT Unadvise(DWORD dwConnection);
		HRESULT SetClientSite(IOleClientSite* pClientSite);
		HRESULT GetClientSite(IOleClientSite** ppClientSite);
		HRESULT GetExtent(DWORD dwDrawAspect, SIZEL* psizel);
		HRESULT SetExtent(DWORD dwDrawAspect, SIZEL* psizel);

		HRESULT SetAdvise(DWORD aspects, DWORD advf, IAdviseSink* pAdvSink);
		HRESULT GetAdvise(DWORD* pAspects, DWORD* pAdvf, IAdviseSink** ppAdvSink);
		HRESULT Draw(
			DWORD dwDrawAspect, LONG lindex,
			void* pvAspect,
			DVTARGETDEVICE* ptd,
			HDC hdcTargetDev,
			HDC hdcDraw,
			LPCRECTL lprcBounds,
			LPCRECTL lprcWBounds,
			BOOL(* pfnContinue)(ULONG_PTR dwContinue),
			ULONG_PTR dwContinue);

		HRESULT GetExtent(
			DWORD dwDrawAspect,
			LONG lindex,
			DVTARGETDEVICE* ptd,
			LPSIZEL lpsizel);
	};
}