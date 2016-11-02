
// hdzb-serverDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "hdzb-server.h"
#include "hdzb-serverDlg.h"
#include "afxdialogex.h"
#include "../src/CustomWinMsg.h"
#include "tim.h"
#include "tim_comm.h"
#include "tim_int.h"
#include "av_common.h"
#include "../src/log4z.h"


using namespace imcore;
using namespace tencent::av;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static HWND g_hWndMain = NULL;

// 定义登录消息回调 

class LoginCallBack : public TIMCallBack 
{
	virtual void OnSuccess()
	{
		::PostMessage(g_hWndMain, WM_ON_LOGIN, (WPARAM)AV_OK, 0);
	}

	virtual void OnError(int retCode, const std::string &desc)
	{
		LOGFMTE("login error. code: %d, desc: %s", retCode, desc.c_str());
		::PostMessage(g_hWndMain, WM_ON_LOGIN, (WPARAM)retCode, NULL);
	}
};



class LogoutCallBack : public TIMCallBack 
{
	virtual void OnSuccess(){::PostMessage(g_hWndMain, WM_ON_LOGOUT, (WPARAM)AV_OK, 0);}
	virtual void OnError(int retCode, const std::string &desc)
	{
		LOGFMTE("login error. code: %d, desc: %s", retCode, desc.c_str());
		::PostMessage(g_hWndMain, WM_ON_LOGOUT, (WPARAM)retCode, retCode);
	}
};


static LoginCallBack s_loginCallback;
static LoginCallBack s_logoutCallback;

// ChdzbserverDlg 对话框

ChdzbserverDlg::ChdzbserverDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(ChdzbserverDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ChdzbserverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ChdzbserverDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_MESSAGE(WM_UI_CUSTOM, OnCustomMsg)
	ON_MESSAGE(WM_ON_LOGIN, OnLogin)
	ON_MESSAGE(WM_ON_LOGOUT, OnLogout)
END_MESSAGE_MAP()


// ChdzbserverDlg 消息处理程序

BOOL ChdzbserverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	ShowWindow(SW_MINIMIZE);
	m_manager = new Manager(this->m_hWnd);
	m_manager->Init();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void ChdzbserverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR ChdzbserverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LONG ChdzbserverDlg::OnCustomMsg( WPARAM wParam, LPARAM lParam )
{
	m_manager->Signal();
	return 0;
}

LONG ChdzbserverDlg::OnLogout( WPARAM wParam, LPARAM lParam )
{
	int retCode = wParam;
	TIMManager::get().Uninit();

	m_manager->OnLogoutResult(retCode);
	return 0;
}

LONG ChdzbserverDlg::OnLogin( WPARAM wParam, LPARAM lParam )
{
	LINE;
	int retCode = wParam;
	if (retCode == AV_OK) {
		m_manager->StartAVSDK();
	} else {
		m_manager->OnLoginResult(retCode);
	}
	return 0;
}

