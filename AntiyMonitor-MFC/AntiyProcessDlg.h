#pragma once


// CAntiyProcessDlg 对话框

class CAntiyProcessDlg : public CDialog
{
	DECLARE_DYNAMIC(CAntiyProcessDlg)

public:
	CAntiyProcessDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CAntiyProcessDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROCESS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ListCtrl;
};
