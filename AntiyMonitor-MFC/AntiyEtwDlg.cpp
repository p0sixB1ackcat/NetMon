// AntiyEtwDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "AntiyMonitor-MFC.h"
#include "AntiyEtwDlg.h"
#include "afxdialogex.h"

// CAntiyEtwDlg �Ի���

IMPLEMENT_DYNAMIC(CAntiyEtwDlg, CDialog)

CAntiyEtwDlg::CAntiyEtwDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_EtwDlg, pParent)
{

}

CAntiyEtwDlg::~CAntiyEtwDlg()
{
}

void CAntiyEtwDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ListCtrl, m_ListCtrl);
}


BEGIN_MESSAGE_MAP(CAntiyEtwDlg, CDialog)
END_MESSAGE_MAP()



