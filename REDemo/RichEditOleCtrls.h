#pragma once
#include "RichEditOleBase.h"
#include <vector>
#include "SImageView.h"

namespace SOUI
{
#pragma region RichEditImageOle
	// RichEditImageOle
	extern "C" const GUID IID_ImageOleCtrl;
	class RichEditImageOle : public RichEditOleBase
	{
		DEF_SOBJECT(RichEditOleBase, L"img")
	public:
		static SStringW TagPath;
		static SStringW MakeFormattedText(const SStringW& imageId,
			const SStringW& subId,
			const SStringW& type,
			const SStringW& skinId,
			const SStringW& imagePath,
			const SStringW& encoding,
			BOOL showManifier);

		RichEditImageOle();
		~RichEditImageOle();

		static RichEditObj* CreateObject() { return new RichEditImageOle(); }

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
		HRESULT STDMETHODCALLTYPE Close(DWORD dwSaveOption);
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

		SStringW GetSelFormatedText();
		void ShowManifier(BOOL show);
		BOOL SetImagePath(const SStringW& path, const SStringW& skinId);
		BOOL SetImageSkin(const SStringW& skin);
		SStringW GetImagePath();
		SStringW GetImageSkin();
		SStringW GetImageType();
		SStringW GetImageUrl();
		void SetImageType(const SStringW& strType);
		SStringW GetEncoding();
		SStringW GetSubId();
		BOOL InitOleWindow(IRichEditObjHost* pHost);
		HRESULT InternalDraw(IRenderTarget* prt, IRegionS* prgn, DWORD cp);
		void PauseImgView() { _pImageView->Pause(); }
		void ResumeImgVidw() { _pImageView->Resume(); }

	protected:
		BOOL SetImageSkin(ISkinObj* pSkin);
		BOOL DownLoadNetworkFile(const SStringW& url, SStringW& path);
		bool OnImageLoaded(SOUI::EventArgs* pEvt);
		LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		SOUI_ATTRS_BEGIN()
			ATTR_SIZE(L"max-size", _maxSize, FALSE)
			ATTR_STRINGW(L"path", _path, FALSE)
			ATTR_STRINGW(L"subid", _subId, FALSE)
			ATTR_STRINGW(L"type", _imageType, FALSE)
			ATTR_STRINGW(L"skin", _skinId, FALSE)
			ATTR_STRINGW(L"encoding", _encoding, FALSE)
			ATTR_STRINGW(L"image_original_url", _imgUrl, FALSE)
			ATTR_INT(L"show-magnifier", _showMagnifier, FALSE)
			SOUI_ATTRS_END()

	private:
		SIZE _maxSize;
		SStringW  _path;
		SStringW _skinId;
		SImageView* _pImageView;
		BOOL _isClosed;
		SStringW _imageType;
		SStringW _encoding;
		SStringW _subId;
		BOOL _showMagnifier;
		SStringW _imgUrl;
	};
#pragma endregion

#pragma region RichEditAudioOle
	extern "C" const GUID IID_AudioOleCtrl;
	class RichEditAudioOle :public RichEditOleBase
	{
		DEF_SOBJECT(RichEditOleBase, L"audio")
	public:
		//长短、状态使用不同皮肤
		enum ReAudioState
		{
			REAUDIO_STATE_NOPLAY,		//	未播放状态
			REAUDIO_STATE_PLAYING,		//  正在播放状态
			REAUDIO_STATE_PLAYFINISH,   //	播放完成状态
		};
		enum ReAudioLen
		{
			REAUDIO_LEN_15,
			REAUDIO_LEN_30,
			REAUDIO_LEN_45,
			REAUDIO_LEN_60,
		};

		RichEditAudioOle();
		~RichEditAudioOle();

		static RichEditObj* CreateObject() { return new RichEditAudioOle(); }


		BOOL InitOleWindow(IRichEditObjHost* pHost);

		int GetAudioState() { return _state; }
		SStringW GetAudioUrl() { return _audio_url; }
		SStringW GetAudioDuration() { return _audio_duration; }
		SStringW GetAudioId() { return GetId(); }
		void AudioPlayFinish();

	protected:
		void UpdatePosition();
		bool OnAudioPlayClick(SOUI::EventArgs* pEvt);
		bool OnAudioPlayingClick(SOUI::EventArgs* pEvt);
	protected:
		SOUI_ATTRS_BEGIN()
			ATTR_STRINGW(L"audio_duration", _audio_duration, FALSE)
			ATTR_STRINGW(L"audio_url", _audio_url, FALSE)
			ATTR_STRINGW(L"audio_layout", _audio_layout, FALSE)
			ATTR_STRINGW(L"audio_isplayed", _audio_isplayed, FALSE)
			SOUI_ATTRS_END()

	private:
		int			_state;
		SIZE        _maxSize;
		SStringW	_audio_duration;
		SStringW	_audio_url;
		SStringW	_audio_layout;
		ReAudioLen	_audio_len;
		SStringW	_audio_isplayed;
	};
#pragma endregion

#pragma region RichEditVideoOle
	extern "C" const GUID IID_VideoOleCtrl;
	class RichEditVideoOle :public RichEditOleBase
	{
		DEF_SOBJECT(RichEditOleBase, L"video")
	public:
		enum ReVideoState
		{
			REVIDEO_STATE_NORMAL,
			REVIDEO_STATE_DOWNLOADING,
			REVIDEO_STATE_DOWNFINISH,
		};

		RichEditVideoOle();
		~RichEditVideoOle();
		static RichEditObj* CreateObject() { return new RichEditVideoOle(); }

		BOOL InitOleWindow(IRichEditObjHost* pHost);
		int GetVideoState() { return _state; }
		SStringW GetVideoUrl() { return _video_url; }
		SStringW GetVideoDuration() { return _video_duration; }
		SStringW GetVideoId() { return GetId(); }
		void SetVideoPlayState(int nstate);
		void SetVideoFrameImage(const SStringW& sstrImagePath);
	protected:
		void UpdatePosition();
		bool OnBnClickPlay(SOUI::EventArgs* pEvt);
	protected:
		SOUI_ATTRS_BEGIN()
			ATTR_STRINGW(L"video_duration", _video_duration, FALSE)
			ATTR_STRINGW(L"video_url", _video_url, FALSE)
			ATTR_STRINGW(L"video_cover_path", _video_cover_path, FALSE)
			ATTR_STRINGW(L"video_cover_width", _video_cover_width, FALSE)
			ATTR_STRINGW(L"video_cover_height", _video_cover_height, FALSE)
			SOUI_ATTRS_END()
	private:
		int			_state;
		SStringW	_video_duration;			//视频时长
		SStringW	_video_url;					//视频文件路径
		SStringW	_video_cover_path;			//视频第一帧图片路径
		SStringW	_video_cover_width;
		SStringW	_video_cover_height;
	};
#pragma endregion

#pragma region RichEditWebOle
	extern "C" const GUID IID_LinkOleCtrl;
	class RichEditLinkOle : public RichEditOleBase
	{
		DEF_SOBJECT(RichEditOleBase, L"link")
	public:
		RichEditLinkOle();
		~RichEditLinkOle();
		static RichEditObj* CreateObject() { return new RichEditLinkOle(); }

		BOOL InitOleWindow(IRichEditObjHost* pHost);
		SStringW GetUrl() { return _link_url; }
	protected:
		void UpdatePosition();
		bool OnBnClickWebOle(SOUI::EventArgs* pEvt);
	protected:
		SOUI_ATTRS_BEGIN()
			ATTR_STRINGW(L"link_title", _link_title, FALSE)
			ATTR_STRINGW(L"link_content", _link_content, FALSE)
			ATTR_STRINGW(L"link_imgpath", _link_imgpath, FALSE)
			ATTR_STRINGW(L"link_url", _link_url, FALSE)
			SOUI_ATTRS_END()
	private:
		SStringW	_link_title;	//链接标题
		SStringW	_link_imgpath;	//链接缩略图路径
		SStringW	_link_content;	//链接简要内容
		SStringW	_link_url;		//链接url
	};
#pragma endregion

#pragma region RichEditLocationOle
	extern "C" const GUID IID_LocationOleCtrl;
	class RichEditLocationOle :public RichEditOleBase
	{
		DEF_SOBJECT(RichEditOleBase, L"location")
	public:
		RichEditLocationOle();
		~RichEditLocationOle();
		static RichEditObj* CreateObject() { return new RichEditLocationOle(); }

		BOOL InitOleWindow(IRichEditObjHost* pHost);

		SStringW GetLocationId() { return GetId(); }
		SStringW GetLongitude() { return _location_longitude; }
		SStringW GetLatitude() { return _location_latitude; }
		void SetLocationImage(const SStringW& sstrImagePath);
		void SetLocationSize(int nWidth, int nHeight);
	protected:
		void UpdatePosition();
		bool OnBnClickLocation(SOUI::EventArgs* pEvt);

	protected:
		SOUI_ATTRS_BEGIN()
			ATTR_STRINGW(L"location_imgpath", _location_imgpath, FALSE)
			ATTR_STRINGW(L"location_title", _location_title, FALSE)
			ATTR_STRINGW(L"location_address", _location_address, FALSE)
			ATTR_STRINGW(L"location_longitude", _location_longitude, FALSE)
			ATTR_STRINGW(L"location_latitude", _location_latitude, FALSE)
			ATTR_STRINGW(L"img_width", _img_width, FALSE)
			ATTR_STRINGW(L"img_height", _img_height, FALSE)
			SOUI_ATTRS_END()

	private:
		SStringW	_location_imgpath;			//地图展示图片
		SStringW	_location_title;			//定位地名
		SStringW	_location_address;			//定位地址
		SStringW	_location_longitude;		//经度
		SStringW	_location_latitude;			//纬度
		SStringW	_img_width;
		SStringW	_img_height;
	};
#pragma endregion


#pragma region RichEditFileOle
	// RichEditFileOle
	extern "C" const GUID IID_FileOleCtrl;
	class RichEditFileOle : public RichEditOleBase
	{
		DEF_SOBJECT(RichEditOleBase, L"file")
	public:
		enum LinkFlag
		{
			LINK_DOWNLOAD = 0x0001,	//下载
			LINK_SAVE = 0x0002,   // 保存按钮
			LINK_SAVEAS = 0x0004,   // 另存为按钮
			LINK_CANCEL = 0x0008,   // 取消传输按钮
			LINK_OPEN_FILE = 0x0010,   // 打开文件按钮
			LINK_OPEN_DIR = 0x0020,   // 打开文件所在文件夹按钮
			LINK_CONTINUE = 0x0040,   // 继续传输按钮
			LINK_FORWARD = 0x0080,   //  转发
		};

		static SStringW MakeFormattedText(
			const SStringW& filePath,
			const SStringW& fileState,
			__int64 fileSize,
			int visibleLinks,
			const SStringW& fileSuffix);

		RichEditFileOle();
		~RichEditFileOle();

		static RichEditObj* CreateObject() { return new RichEditFileOle(); }

		virtual BOOL InitOleWindow(IRichEditObjHost* pHost);

		void SetFilePath(const SStringW& path);
		void SetFileLinksVisible(int links);
		void SetFileSize(__int64 size, BOOL requestLayout = TRUE);
		void SetFileStateString(const SStringW& str);
		SStringW GetFilePath();
		SStringW GetFileName();
		__int64 GetFileSize();
		SStringW GetFileSuffix();
		void SetFileSuffix(const SStringW& suffix);
		void SetFileUrl(const SStringW& url);
		SStringW GetFileUrl();
		SStringW GetFileOLeId() { return GetId(); }

	protected:
		bool OnLinkClicked(SOUI::IEvtArgs* pEvt);
		bool OnFileNameClicked(SOUI::IEvtArgs* pEvt);

		SOUI_ATTRS_BEGIN()
			ATTR_STRINGW(L"file-path", _filePath, FALSE)
			ATTR_STRINGW(L"file-size", _fileSize, FALSE)
			ATTR_STRINGW(L"file-state", _fileState, FALSE)
			ATTR_INT(L"links", _links, FALSE)
			ATTR_STRINGW(L"file-suffix", _fileSuffix, FALSE)
			ATTR_STRINGW(L"file-url", _fileUrl, FALSE)
			SOUI_ATTRS_END()

			SStringW GetSizeBeautyString(unsigned long long size);

	private:
		SStringW _filePath;         // 文件路径
		SStringW _fileName;         // 文件名
		SStringW _fileSize;         // 文件尺寸
		SStringW _fileState;        // 文件状态
		__int64  _fileSizeBytes;    // 整形的文件大小
		int      _links;            // 可见按钮的标记，见LinkFlag
		SStringW _fileSuffix;
		SStringW _fileUrl;
	};

	// RichEditFileOle inlines
	inline SStringW RichEditFileOle::GetFilePath()
	{
		return _filePath;
	}

	inline SStringW RichEditFileOle::GetFileName()
	{
		return _fileName;
	}

	inline SStringW RichEditFileOle::GetFileSuffix()
	{
		return _fileSuffix;
	}
#pragma endregion

#pragma region RichEditFetchMoreOle
	extern "C" const GUID IID_FetchMoreOleCtrl;
	class RichEditFetchMoreOle : public RichEditOleBase
	{
		DEF_SOBJECT(RichEditOleBase, L"fetchmore")
	public:
		enum FetchMoreState
		{
			REFM_STATE_NORMAL,  // 正常状态，显示获取更多
			REFM_STATE_LOADING, // 正在加载状态，显示转圈圈
			REFM_STATE_END,     // 显示更多消息请在历史记录中查询
		};
		RichEditFetchMoreOle();

		static RichEditObj* CreateObject() { return new RichEditFetchMoreOle(); }

		BOOL    InitOleWindow(IRichEditObjHost* pHost);
		void    ResetState();
		void    ShowLoadingState();
		void    ShowOpenLinkState();
		void    HideOle();
		//void    Subscribe(const ISlotFunctor& subscriber);
		int     GetCurrentState() { return _state; }

	protected:
		bool    OnClickFetchMore(SOUI::EventArgs* pEvt);
		bool    OnClickOpenHistory(SOUI::EventArgs* pEvt);
		//LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		void    UpdatePosition();

	private:
		int _state;
	};
#pragma endregion

#pragma region RichEditSeparatorBar
	// RichEditSeparatorBar
	extern "C" const GUID IID_SeparatorBarCtrl;
	class RichEditSeparatorBar : public RichEditOleBase
	{
		DEF_SOBJECT(RichEditOleBase, L"split")
	public:
		RichEditSeparatorBar();

		static RichEditObj* CreateObject() { return new RichEditSeparatorBar(); }

	protected:
		void    UpdatePosition();
	};
#pragma endregion

#pragma region RichEditReminderOle
	extern "C" const GUID IID_RemainderOleCtrl;
	class RichEditReminderOle : public RichEditOleBase
	{
		DEF_SOBJECT(RichEditOleBase, L"remainder")
	public:
		static SStringW MakeFormattedText(const SStringW& text,
			int textSize,
			COLORREF textColor);
		RichEditReminderOle();
		static RichEditObj* CreateObject() { return new RichEditReminderOle(); }

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
		BOOL        InitFromXml(IXmlNode* pNode);
		BOOL        InitOleWindow(IRichEditObjHost* pHost);
		SStringW    GetText() { return _text; }
		SStringW    GetSelFormatedText();

	protected:
		void    CalculateNatureSize();
		void    InitAttributes();
		LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		SOUI_ATTRS_BEGIN()
			ATTR_COLOR(L"color-border", _borderColor, FALSE)
			ATTR_COLOR(L"color-bkgnd", _bkCorlor, FALSE)
			ATTR_COLOR(L"color-text", _textColor, FALSE)
			ATTR_INT(L"border-width", _borderWidth, FALSE)
			ATTR_STRINGW(L"font", _font, FALSE)
			ATTR_SIZE(L"max-size", _maxSize, FALSE)
			ATTR_INT(L"height", _height, FALSE)

			ATTR_STRINGW(L"cursor", _cursor, FALSE)
			SOUI_ATTRS_END()
	private:
		COLORREF    _borderColor;
		COLORREF    _bkCorlor;
		COLORREF    _textColor;
		int         _borderWidth;
		SStringW    _text;
		SStringW    _font;
		SIZE        _maxSize;
		int         _height;

		SStringW    _cursor;
	};
#pragma endregion

#pragma region RichMetaFile
	extern "C" const GUID IID_RichMetaFileOle;
	class RichEditMetaFileOle : public RichEditOleBase
	{
#define OLE_MAX_WIDTH 120
#define OLE_MIN_WIDTH 45
#define OLE_HEIGHT    65

		DEF_SOBJECT(RichEditOleBase, L"metafile")
	public:
		RichEditMetaFileOle();
		static RichEditObj* CreateObject() { return new RichEditMetaFileOle(); }

		virtual SStringW GetSelFormatedText();
		virtual BOOL InitOleWindow(IRichEditObjHost* pHost);
		void SetFilePath(LPCWSTR lpszFileName);
		SStringW GetFilePath();

	protected:
		void    SetFileName(LPCWSTR lpszFileName);
		void    CalculateNatureSize(const SStringW& FileName);
		BOOL    LoadFileIcon();
		bool    OnOleWindowEvent(SOUI::EventArgs* pEvt);
		LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		SOUI_ATTRS_BEGIN()
			ATTR_STRINGW(L"file", _filePath, FALSE)
			ATTR_STRINGW(L"font", _font, FALSE)
			SOUI_ATTRS_END()

	private:
		SStringW _filePath;
		SStringW _font;
	};
#pragma endregion

#pragma region RevokeOle
	extern "C" const GUID IID_ReEditorOleCtrl;
	class RichEditReEditorOle : public RichEditOleBase
	{
		DEF_SOBJECT(RichEditOleBase, L"re_editor")
	public:
		RichEditReEditorOle();
		~RichEditReEditorOle();

		static RichEditObj* CreateObject() { return new RichEditReEditorOle(); }

		virtual BOOL InitOleWindow(IRichEditObjHost* pHost);

		void ShowLink(BOOL bShow);

		SStringW GetRevokeContent() { return _revokecontent; }
	protected:
		bool OnLinkClicked(SOUI::EventArgs* pEvt);

		SOUI_ATTRS_BEGIN()
			ATTR_STRINGW(L"reeditor_id", _revokeid, FALSE)
			ATTR_STRINGW(L"reeditor_content", _revokecontent, FALSE)
			SOUI_ATTRS_END()
	private:

		SStringW _revokeid;
		SStringW _revokecontent;
	};
#pragma endregion


}


