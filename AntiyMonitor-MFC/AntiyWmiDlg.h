#pragma once
#include "afxwin.h"


// CAntiyWmiDlg 对话框

class CAntiyWmiDlg : public CDialog
{
	DECLARE_DYNAMIC(CAntiyWmiDlg)

public:
	CAntiyWmiDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAntiyWmiDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WMIDlg };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	
	CEdit m_WmiEdit;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton5();
	void HideControlDriverBtn();
	CButton m_InstallButton;
	CButton m_StartButton;
	CButton m_StopButton;
	CButton m_DeleteButton;
	CButton m_CreatePortButton;
};
