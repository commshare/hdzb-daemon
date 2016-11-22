
// hdzb-serverDlg.h : ͷ�ļ�
//

#pragma once

#include "../src/Manager.h"

// ChdzbserverDlg �Ի���
class ChdzbserverDlg : public CDialogEx
{
// ����
public:
	ChdzbserverDlg(CWnd* pParent = NULL);	// ��׼���캯��  

// �Ի�������
	enum { IDD = IDD_HDZBSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
