
// hdzb-serverDlg.cpp : ʵ���ļ�
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

// �����¼��Ϣ�ص� 

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

// ChdzbserverDlg �Ի���

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


// ChdzbserverDlg ��Ϣ�������

BOOL ChdzbserverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	ShowWindow(SW_MINIMIZE);
	m_manager = new Manager(this->m_hWnd);
	m_manager->Init();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void ChdzbserverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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

