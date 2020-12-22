
// AntiyMonitor-MFCDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "AntiyEtwDlg.h"
#include "AntiyWmiDlg.h"
#include "AntiyProcessDlg.h"

// CAntiyMonitorMFCDlg 对话框
class CAntiyMonitorMFCDlg : public CDialogEx
{
// 构造
public:
	CAntiyMonitorMFCDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ANTIYMONITORMFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
