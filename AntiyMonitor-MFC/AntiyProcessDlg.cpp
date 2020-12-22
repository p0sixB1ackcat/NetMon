// CAntiyProcessDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "AntiyMonitor-MFC.h"
#include "AntiyProcessDlg.h"
#include "afxdialogex.h"


// CAntiyProcessDlg 对话框

IMPLEMENT_DYNAMIC(CAntiyProcessDlg, CDialog)

CAntiyProcessDlg::CAntiyProcessDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_PROCESS, pParent)
{

}

CAntiyProcessDlg::~CAntiyProcessDlg()
{
}

void CAntiyProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTL, m_ListCtrl);
}


BEGIN_MESSAGE_MAP(CAntiyProcessDlg, CDialog)
END_MESSAGE_MAP()


// CAntiyProcessDlg 消息处理程序
