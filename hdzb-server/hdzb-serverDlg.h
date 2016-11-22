
// hdzb-serverDlg.h : 头文件
//

#pragma once

#include "../src/Manager.h"

// ChdzbserverDlg 对话框
class ChdzbserverDlg : public CDialogEx
{
// 构造
public:
	ChdzbserverDlg(CWnd* pParent = NULL);	// 标准构造函数  

// 对话框数据
	enum { IDD = IDD_HDZBSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg LONG OnCustomMsg(WPARAM wParam, LPARAM lParam); 
	afx_msg LONG OnLogout(WPARAM wParam, LPARAM lParam); 
	afx_msg LONG OnLogin(WPARAM wParam, LPARAM lParam); 
	afx_msg LONG OnPushVideoStart(WPARAM wParam, LPARAM lParam); 
	afx_msg LONG OnPushVideoStop(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	Manager* m_manager;
};
