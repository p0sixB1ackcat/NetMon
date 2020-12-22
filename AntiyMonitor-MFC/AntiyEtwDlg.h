#pragma once
#include "afxcmn.h"


// CAntiyEtwDlg 对话框

class CAntiyEtwDlg : public CDialog
{
	DECLARE_DYNAMIC(CAntiyEtwDlg)

public:
	CAntiyEtwDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAntiyEtwDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EtwDlg };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ListCtrl;
};
