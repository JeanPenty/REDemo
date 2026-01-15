// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include <helper/SMenuEx.h>

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	
}

CMainDlg::~CMainDlg()
{
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	SImRichEdit* pRecvEdit = FindChildByName2<SImRichEdit>(L"recv_richedit");
	if (!pRecvEdit)
		return 0;

	std::ostringstream os;
	//统一使用右侧布局
	LPCWSTR pEmpty;
	pEmpty = L"<para id=\"msgbody\" margin=\"0,0,0,0\" break=\"1\" simulate-align=\"1\">"
		L""
		L"</para>";

	//插入消息时间
	time_t tt = time(NULL);
	tm* t = localtime(&tt);
	SStringW sstrTempTime;
	if (t->tm_hour > 12)
		sstrTempTime.Format(L"%02d月%02d日  下午%02d:%02d", t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
	else
		sstrTempTime.Format(L"%02d月%02d日  上午%02d:%02d", t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
	SStringW sstrTime;
	sstrTime.Format(L"<text font-size=\"8\" color=\"#808080\">%s</text>", sstrTempTime.c_str());

	SStringW sstrTimeContent;
	sstrTimeContent.Format(
		L"<RichEditContent type=\"ContentCenter\" >"
		L"<para margin=\"0,10,0,5\" align=\"center\" break=\"1\" >"
		L"%s"
		L"</para>"
		L"</RichEditContent>", sstrTime.c_str());
	pRecvEdit->InsertContent(sstrTimeContent, RECONTENT_LAST);


// 	SStringW sstrResend;
// 	sstrResend = L"<bkele id=\"resend\" name=\"BkEleSendFail\" data=\"resend\" right-skin=\"skin.richedit_resend\" right-pos=\"{-15,[-16,@12,@12\" cursor=\"hand\" interactive=\"1\"/>";
// 
// 	SStringW sstrMsg = L"ceshi";
// 
// 	SStringW sstrContent1;
// 	sstrContent1.Format(
// 		L"<RichEditContent  type=\"ContentRight\" align=\"right\" auto-layout=\"1\">"
// 		L"<para break=\"1\" align=\"left\" />"
// 		L"<bkele data=\"avatar\" id=\"%s\" skin=\"%s\" left-pos=\"0,]-6,@40,@40\" right-pos=\"-50,]-9,@40,@40\" cursor=\"hand\" interactive=\"1\"/>"
// 		L"<para id=\"msgbody\" margin=\"65,0,35,0\" break=\"1\" simulate-align=\"1\">"
// 		L"%s"
// 		L"</para>"
// 		L"<bkele data=\"bubble\" left-skin=\"skin_left_bubble\" right-skin=\"skin_right_otherbubble\" left-pos=\"50,{-9,[10,[10\" right-pos=\"{-10,{-9,-55,[10\" />"
// 		L"%s"
// 		L"</RichEditContent>",
// 		L"default_portrait", L"default_portrait", sstrMsg, pEmpty);

// 	SStringW sstrContent;
// 	sstrContent.Format(
// 		L"<RichEditContent  type=\"ContentRight\" align=\"right\" auto-layout=\"1\">"
// 		L"<para break=\"1\" align=\"left\" />"
// 		L"<bkele data=\"avatar\" id=\"%s\" skin=\"%s\" left-pos=\"0,]-6,@40,@40\" right-pos=\"-50,]-9,@40,@40\" cursor=\"hand\" interactive=\"1\"/>"
// 		L"<para id=\"msgbody\" margin=\"65,0,35,0\" break=\"1\" simulate-align=\"1\">"
// 		L"%s"
// 		L"</para>"
// 		L"<bkele data=\"bubble\" left-skin=\"skin_left_bubble\" right-skin=\"skin_right_otherbubble\" left-pos=\"50,{-9,[10,[10\" right-pos=\"{-10,{-9,-55,[10\" />"
// 		L"%s"
// 		L"</RichEditContent>",
// 		L"default_portrait", L"default_portrait", sstrContent1, pEmpty);

// 	pRecvEdit->InsertContent(sstrContent, RECONTENT_LAST);
// 	pRecvEdit->ScrollToBottom();

	SStringW sstrMessageID = L"1234567898";
	SStringW sstrContent = SStringW().Format(
		L"<RichEditContent msgtype=\"text\" id=\"%s\" content_id=\"%s\" talk_type=\"group\" origin=\"%s\" timestamp=\"%s\" type=\"ContentLeft\" align=\"left\" auto-layout=\"1\">"
		L"<para id=\"msgbody\" margin=\"20,10,10,10\" break=\"1\" simulate-align=\"1\">"
		L"<text font-size=\"10\" font-face=\"微软雅黑\" color=\"#000000\">测试1234561234 测试1234561234</text>"
		L"</para>"
		L"<bkele data=\"bubble\" left-skin=\"skin_left_bubble\" right-skin=\"skin_right_otherbubble\" left-pos=\"50,10,100,50\" right-pos=\"-10,10,100,50\" />"
		L"</RichEditContent>",
		sstrMessageID, sstrMessageID, L"", L"");
	
 	pRecvEdit->InsertContent(sstrContent, RECONTENT_LAST);

	return 0;
}
//TODO:消息映射
void CMainDlg::OnClose()
{
	SNativeWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

