#pragma once
#include "afxcmn.h"


// CAntiyEtwDlg �Ի���

class CAntiyEtwDlg : public CDialog
{
	DECLARE_DYNAMIC(CAntiyEtwDlg)

public:
	CAntiyEtwDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAntiyEtwDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EtwDlg };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ListCtrl;
};
