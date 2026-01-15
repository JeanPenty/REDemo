#pragma once

#define GETSKIN(p1,scale) SSkinPoolMgr::getSingleton().GetSkin(p1,scale)
#define GETBUILTINSKIN(p1) SSkinPoolMgr::getSingleton().GetBuiltinSkin(p1,100)
#define GETBUILTINSKIN2(p1,scale) SSkinPoolMgr::getSingleton().GetBuiltinSkin(p1,scale)
#define GETSKINPOOLMGR SSkinPoolMgr::getSingletonPtr()

namespace SOUI
{
	class SSkinPoolMgr : public SSingleton<SSkinPoolMgr>
	{
	public:
		SSkinPoolMgr();
		~SSkinPoolMgr();

		/**
		* GetSkin
		* @brief    获得与指定name匹配的SkinObj
		* @param    LPCWSTR strSkinName --    Name of Skin Object
		* @return   ISkinObj*  -- 找到的Skin Object
		* Describe
		*/
		ISkinObj* GetSkin(const SStringW& strSkinName, int nScale);

		/**
		 * PushSkinPool
		 * @brief    向列表中增加一个新的SSkinPool对象
		 * @param    SSkinPool * pSkinPool --    SSkinPool对象
		 * @return   void
		 * Describe
		 */
		void PushSkinPool(SSkinPool* pSkinPool);

		/**
		 * PopSkinPool
		 * @brief    弹出一个SSkinPool对象
		 * @param    SSkinPool * pSkinPool --   准备弹出的SSkinPool对象
		 * @return   SSkinPool *    在列表中找到后弹出的SSkinPool对象
		 * Describe  内建SkinPool不用调用PopSkinPool
		 */
		SSkinPool* PopSkinPool(SSkinPool* pSkinPool);

		/**
		 * GetBuiltinSkin
		 * @brief    获得SOUI系统内建的命名SkinObj
		 * @param    SYS_SKIN uID --  内建SKIN的ID
		 * @return   ISkinObj * 与SKINID对应的ISkinObj
		 * Describe  可能返回失败
		 */
		ISkinObj* GetBuiltinSkin(SYS_SKIN uID, int nScale);

		/**
		 * GetBuiltinSkinPool
		 * @brief    获得管理内建SkinPool对象
		 * @return   SSkinPool * -- 内建SkinPool指针
		 * Describe  用户在代码中创建的SkinObj可以交给内建SkinPool管理
		 */
		SSkinPool* GetBuiltinSkinPool() { return m_bulitinSkinPool; }

		void SetBuiltinSkinPool(SSkinPool* pSkinPool) {
			m_bulitinSkinPool = pSkinPool;
		}
	protected:
		SList<SSkinPool*> m_lstSkinPools;
		CAutoRefPtr<SSkinPool> m_bulitinSkinPool;
	};
}


