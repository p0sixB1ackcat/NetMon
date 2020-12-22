// AntiyWmiDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AntiyMonitor-MFC.h"
#include "AntiyWmiDlg.h"
#include "afxdialogex.h"
#include "AntiyMiniFilter.h"
#include "AntiyMonUK.h"

// CAntiyWmiDlg 对话框

IMPLEMENT_DYNAMIC(CAntiyWmiDlg, CDialog)

CAntiyWmiDlg::CAntiyWmiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_WMIDlg, pParent)
{

}

CAntiyWmiDlg::~CAntiyWmiDlg()
{
	
}

void CAntiyWmiDlg::HideControlDriverBtn(void)
{
	m_InstallButton.ShowWindow(FALSE);
	m_StartButton.ShowWindow(FALSE);
	m_StopButton.ShowWindow(FALSE);
	m_DeleteButton.ShowWindow(FALSE);
	m_CreatePortButton.ShowWindow(FALSE);
}

void CAntiyWmiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, m_WmiEdit);
	DDX_Control(pDX, IDC_BUTTON1, m_InstallButton);
	DDX_Control(pDX, IDC_BUTTON4, m_StartButton);
	DDX_Control(pDX, IDC_BUTTON3, m_StopButton);
	DDX_Control(pDX, IDC_BUTTON2, m_DeleteButton);
	DDX_Control(pDX, IDC_BUTTON5, m_CreatePortButton);
}


BEGIN_MESSAGE_MAP(CAntiyWmiDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CAntiyWmiDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON4, &CAntiyWmiDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON3, &CAntiyWmiDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, &CAntiyWmiDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON5, &CAntiyWmiDlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CAntiyWmiDlg 消息处理程序


void CAntiyWmiDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (CAntiyMiniFilter::ShareInstance()->InstallMiniFilter())
	{
		MessageBox(L"安装成功",L"message");
	}
	else
	{
		MessageBox(L"安装失败", L"message");
	}
}


void CAntiyWmiDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	if (CAntiyMiniFilter::ShareInstance()->StartFilter())
	{
		MessageBox(L"启动成功", L"message");
	}
	else
	{
		MessageBox(L"启动失败", L"message");
	}
}


void CAntiyWmiDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	if (CAntiyMiniFilter::ShareInstance()->StopFilter())
	{
		MessageBox(L"停止成功", L"message");
	}
	else
	{
		MessageBox(L"停止失败", L"message");
	}
}


void CAntiyWmiDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (CAntiyMiniFilter::ShareInstance()->UnInstallMiniFilter())
	{
		MessageBox(L"卸载成功", L"message");
	}
	else
	{
		MessageBox(L"卸载失败", L"message");
	}
}

void CAntiyWmiDlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	if (CAntiyMiniFilter::ShareInstance()->CreatePort(ATPortName))
	{
		MessageBox(L"开启监控成功", L"message");
	}
	else
	{
		MessageBox(L"开启监控失败", L"message");
	}

}
