// AntiyWmiDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "AntiyMonitor-MFC.h"
#include "AntiyWmiDlg.h"
#include "afxdialogex.h"
#include "AntiyMiniFilter.h"
#include "AntiyMonUK.h"

// CAntiyWmiDlg �Ի���

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


// CAntiyWmiDlg ��Ϣ�������


void CAntiyWmiDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (CAntiyMiniFilter::ShareInstance()->InstallMiniFilter())
	{
		MessageBox(L"��װ�ɹ�",L"message");
	}
	else
	{
		MessageBox(L"��װʧ��", L"message");
	}
}


void CAntiyWmiDlg::OnBnClickedButton4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (CAntiyMiniFilter::ShareInstance()->StartFilter())
	{
		MessageBox(L"�����ɹ�", L"message");
	}
	else
	{
		MessageBox(L"����ʧ��", L"message");
	}
}


void CAntiyWmiDlg::OnBnClickedButton3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (CAntiyMiniFilter::ShareInstance()->StopFilter())
	{
		MessageBox(L"ֹͣ�ɹ�", L"message");
	}
	else
	{
		MessageBox(L"ֹͣʧ��", L"message");
	}
}


void CAntiyWmiDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (CAntiyMiniFilter::ShareInstance()->UnInstallMiniFilter())
	{
		MessageBox(L"ж�سɹ�", L"message");
	}
	else
	{
		MessageBox(L"ж��ʧ��", L"message");
	}
}

void CAntiyWmiDlg::OnBnClickedButton5()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (CAntiyMiniFilter::ShareInstance()->CreatePort(ATPortName))
	{
		MessageBox(L"������سɹ�", L"message");
	}
	else
	{
		MessageBox(L"�������ʧ��", L"message");
	}

}
