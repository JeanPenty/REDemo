#include "stdafx.h"
#include "SSkinPoolMgr.h"

#include "res.mgr/SSkinPool.h"
#include "core/Sskin.h"
#include "SApp.h"
#include "helper/SAutoBuf.h"

namespace SOUI
{
	template<> SSkinPoolMgr* SSingleton<SSkinPoolMgr>::ms_Singleton = 0;
	SSkinPoolMgr::SSkinPoolMgr()
	{
		m_bulitinSkinPool.Attach(new SSkinPool);
		PushSkinPool(m_bulitinSkinPool);
	}

	SSkinPoolMgr::~SSkinPoolMgr()
	{
		SPOSITION pos = m_lstSkinPools.GetHeadPosition();
		while (pos)
		{
			SSkinPool* p = m_lstSkinPools.GetNext(pos);
			p->Release();
		}
		m_lstSkinPools.RemoveAll();

	}

	ISkinObj* SSkinPoolMgr::GetSkin(const SStringW& strSkinName, int nScale)
	{
		SPOSITION pos = m_lstSkinPools.GetTailPosition();
		while (pos)
		{
			SSkinPool* pSkinPool = m_lstSkinPools.GetPrev(pos);
			if (ISkinObj* pSkin = pSkinPool->GetSkin(strSkinName, nScale))
			{
				return pSkin;
			}
		}

		if (wcscmp(strSkinName, L"") != 0)
		{
			SASSERT_FMTW(FALSE, L"GetSkin[%s] Failed!", strSkinName);
		}
		return NULL;
	}

	const wchar_t* BUILDIN_SKIN_NAMES[] =
	{
		L"_skin.sys.checkbox",
		L"_skin.sys.radio",
		L"_skin.sys.focuscheckbox",
		L"_skin.sys.focusradio",
		L"_skin.sys.btn.normal",
		L"_skin.sys.scrollbar",
		L"_skin.sys.border",
		L"_skin.sys.dropbtn",
		L"_skin.sys.tree.toggle",
		L"_skin.sys.tree.checkbox",
		L"_skin.sys.tab.page",
		L"_skin.sys.header",
		L"_skin.sys.split.vert",
		L"_skin.sys.split.horz",
		L"_skin.sys.prog.bkgnd",
		L"_skin.sys.prog.bar",
		L"_skin.sys.vert.prog.bkgnd",
		L"_skin.sys.vert.prog.bar",
		L"_skin.sys.slider.thumb",
		L"_skin.sys.btn.close",
		L"_skin.sys.btn.minimize",
		L"_skin.sys.btn.maxmize",
		L"_skin.sys.btn.restore",
		L"_skin.sys.menu.check",
		L"_skin.sys.menu.sep",
		L"_skin.sys.menu.arrow",
		L"_skin.sys.menu.border",
		L"_skin.sys.menu.skin",
		L"_skin.sys.icons",
		L"_skin.sys.wnd.bkgnd"
	};


	ISkinObj* SSkinPoolMgr::GetBuiltinSkin(SYS_SKIN uID, int nScale)
	{
		return GetBuiltinSkinPool()->GetSkin(BUILDIN_SKIN_NAMES[uID], nScale);
	}

	void SSkinPoolMgr::PushSkinPool(SSkinPool* pSkinPool)
	{
		m_lstSkinPools.AddTail(pSkinPool);
		pSkinPool->AddRef();
	}

	SSkinPool* SSkinPoolMgr::PopSkinPool(SSkinPool* pSkinPool)
	{
		SSkinPool* pRet = NULL;
		if (pSkinPool)
		{
			if (pSkinPool == m_bulitinSkinPool) return NULL;

			SPOSITION pos = m_lstSkinPools.Find(pSkinPool);
			if (pos)
			{
				pRet = m_lstSkinPools.GetAt(pos);
				m_lstSkinPools.RemoveAt(pos);
			}
		}
		else
		{
			pRet = m_lstSkinPools.RemoveTail();
		}
		if (pRet) pRet->Release();
		return pRet;
	}
}