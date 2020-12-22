
// AntiyMonitor-MFCDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "AntiyEtwDlg.h"
#include "AntiyWmiDlg.h"
#include "AntiyProcessDlg.h"

// CAntiyMonitorMFCDlg �Ի���
class CAntiyMonitorMFCDlg : public CDialogEx
{
// ����
public:
	CAntiyMonitorMFCDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ANTIYMONITORMFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	virtual LRESULT UpdateDatas(WPARAM wParam, LPARAM lParam);
	static void CheckData(void);
	static void CAntiyMonitorMFCDlg::ShowProcessInfo(void *p);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTabCtrlSelectChange(NMHDR *pNMHDR, LRESULT *pResult);
	CTabCtrl m_TabCtrl;
	CAntiyWmiDlg m_WmiView;
	CAntiyEtwDlg m_EtwView;
	CAntiyProcessDlg m_ProcessView;
#define MAX_TAB_ITEM 3
	CDialog *m_pTabItems[MAX_TAB_ITEM];
	int m_CurSelTab;
	afx_msg void OnTcnSelchangeTabctrl(NMHDR *pNMHDR, LRESULT *pResult);
};
