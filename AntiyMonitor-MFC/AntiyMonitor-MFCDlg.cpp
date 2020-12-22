
// AntiyMonitor-MFCDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "AntiyMonitor-MFC.h"
#include "AntiyMonitor-MFCDlg.h"
#include "afxdialogex.h"
#include "AntiyWmi.h"
#include "AntiyMiniFilter.h"
#include "AntiyMonReport.h"
#include "AntiyEtw.h"
#include "AntiyResource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TITLECOUNT 7


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAntiyMonitorMFCDlg �Ի���

CAntiyMonitorMFCDlg::CAntiyMonitorMFCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ANTIYMONITORMFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAntiyMonitorMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TabCtrl, m_TabCtrl);
}

BEGIN_MESSAGE_MAP(CAntiyMonitorMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TabCtrl, &CAntiyMonitorMFCDlg::OnTabCtrlSelectChange)
	ON_MESSAGE(WM_UPDATEETWDATA, CAntiyMonitorMFCDlg::UpdateDatas)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


int
WINAPIV
w_wsprintf(
	_Out_ LPWSTR sBuffer,
	_In_ _Printf_format_string_ LPCWSTR FormatStr,
	...)
{
	int i = 0;
	WCHAR *buf = NULL;

	va_list va;

	va_start(va, sBuffer);
	buf = va_arg(va,WCHAR *);
	buf = va_arg(va, WCHAR *);
	if (buf)
	{
		wsprintf(sBuffer,FormatStr,buf);
	}

	va_end(va);
	
	return 0;
}

LRESULT CAntiyMonitorMFCDlg::UpdateDatas(WPARAM wParam, LPARAM lParam)
{
	int i;
	
	if (lParam == UDDM_ETW)
	{
		int iLine = m_EtwView.m_ListCtrl.GetItemCount();
		if (iLine > 30)
		{
			m_EtwView.m_ListCtrl.DeleteAllItems();
            iLine = 0;
		}

		m_EtwView.m_ListCtrl.InsertItem(iLine, CAntiyEventTraceData::ShareInstance()->m_Time);
		m_EtwView.m_ListCtrl.SetItemText(iLine, 0, CAntiyEventTraceData::ShareInstance()->m_Time);
		m_EtwView.m_ListCtrl.SetItemText(iLine, 1, CAntiyEventTraceData::ShareInstance()->m_ProcessName);
		m_EtwView.m_ListCtrl.SetItemText(iLine, 2, CAntiyEventTraceData::ShareInstance()->m_ProcessId);
		m_EtwView.m_ListCtrl.SetItemText(iLine, 3, CAntiyEventTraceData::ShareInstance()->m_OperType);
		m_EtwView.m_ListCtrl.SetItemText(iLine, 4, CAntiyEventTraceData::ShareInstance()->m_OperationPath);
		m_EtwView.m_ListCtrl.SetItemText(iLine, 5, CAntiyEventTraceData::ShareInstance()->m_OperaResult);

		if (iLine > 0)
			m_EtwView.m_ListCtrl.EnsureVisible(iLine - 1, FALSE);

		UpdateData(FALSE);
	}
	else if (lParam == UDDM_WMI)
	{
		PREPORT_WMI_EVENT pWmiEventData = (PREPORT_WMI_EVENT)wParam;
		WCHAR pBuffer[4096] = {0};

		w_wsprintf(pBuffer,L"{\r\n");
		
		w_wsprintf(pBuffer + (wcslen(pBuffer)),L"\r\tUTC_TIME = %ws\r\n", pWmiEventData->szTime);
		w_wsprintf(pBuffer + (wcslen(pBuffer)),L"\r\tPid = %d\r\n", pWmiEventData->dwProcessId);
		w_wsprintf(pBuffer + (wcslen(pBuffer)),L"\r\tProcess Path = %ws\r\n", pWmiEventData->szProcessFullPath);
		w_wsprintf(pBuffer + wcslen(pBuffer),L"\r\tEvetName = %ws\r\n",pWmiEventData->szEventName);
		w_wsprintf(pBuffer + (wcslen(pBuffer)),L"\r\tServer = %ws\r\n", pWmiEventData->szServerName);
		w_wsprintf(pBuffer + (wcslen(pBuffer)),L"\r\tClass Name = %ws\r\n", pWmiEventData->szClassName);
		w_wsprintf(pBuffer + (wcslen(pBuffer)), L"\r\tNamespace = %ws\r\n", pWmiEventData->szNamespace);
		w_wsprintf(pBuffer + (wcslen(pBuffer)), L"\r\tName = %ws\r\n", pWmiEventData->szName);
		w_wsprintf(pBuffer + (wcslen(pBuffer)), L"\r\tQuery = %ws\r\n", pWmiEventData->szQuery);
		w_wsprintf(pBuffer + (wcslen(pBuffer)), L"\r\tQueryLanguage = %ws\r\n", pWmiEventData->szQueryLanguage);
		
		for (i = 0; i < pWmiEventData->OperaCount; ++i)
		{
			if(pWmiEventData->OperaList[i] == NULL)
				continue;
			w_wsprintf(pBuffer + (wcslen(pBuffer)), L"\r\n\r\t{\r\n");

			w_wsprintf(pBuffer + (wcslen(pBuffer)), L"\r\t\r\t%ws", pWmiEventData->OperaList[i]);

			w_wsprintf(pBuffer + (wcslen(pBuffer)), L"\r\n\r\t}");
		}
		wsprintf(pBuffer + (wcslen(pBuffer)), L"\r\n}\r\n");
		
		m_WmiView.m_WmiEdit.SetWindowTextW((LPCTSTR)pBuffer);
		UpdateData(FALSE);
	}
	else if (lParam == UDDM_CREATEPROCESS)
	{
		REPORT_MFC_DATA *p = (REPORT_MFC_DATA *)wParam;
		SYSTEMTIME st = {0x00};
		
		if (!p)
			return 0;
		int iLine = m_ProcessView.m_ListCtrl.GetItemCount();
		if (iLine > 30)
		{
			m_ProcessView.m_ListCtrl.DeleteAllItems();
		}
		m_ProcessView.m_ListCtrl.InsertItem(iLine,L"title");
		WCHAR CopyBuffer[MAX_PATH] = {0};
		swprintf_s(CopyBuffer,L"%d",p->ProcessId);
		m_ProcessView.m_ListCtrl.SetItemText(iLine,0, CopyBuffer);
		GetLocalSystemTimeByFileTimeStamp(&p->TimeStamp,&st);
		swprintf_s(CopyBuffer, L"%04d-%02d-%02d %02d:%02d:%02d",st.wYear,st.wMonth,st.wDay, st.wHour, st.wMinute, st.wSecond);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 1, CopyBuffer);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 2, p->CommandLine);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 3, p->ImagePath);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 4, p->CurrentDirtory);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 5, p->Local_File_Info.m_FileVersion);
		m_ProcessView.m_ListCtrl.SetItemText(iLine,6,p->Local_File_Info.m_Description);
		m_ProcessView.m_ListCtrl.SetItemText(iLine,7,p->Local_File_Info.m_ProductName);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 8, p->Local_File_Info.m_Company);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 9, p->ParentCommandLine);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 10, p->ParentImagePath);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 11, p->ParentCurrentDirtyory);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 12, p->Local_File_Info.m_MD5);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 13, p->Local_File_Info.m_Sha1);
		m_ProcessView.m_ListCtrl.SetItemText(iLine, 14, p->Local_File_Info.m_Sha256);

		UpdateData(FALSE);
	}
	else if (lParam == UDDM_TERMINPROC)
	{

	}

	return 0;
}

void CAntiyMonitorMFCDlg::CheckData(void)
{
	//�������ڷ�����Ϣ�������̸߳�������
	//SendMessage(WM_UPDATEETWDATA,FALSE,FALSE);
	HWND window = ::FindWindow(NULL, L"AntiyMonitor-MFC");
	::SendMessage(window, WM_UPDATEETWDATA, FALSE, UDDM_ETW);
	
}

void CAntiyMonitorMFCDlg::ShowProcessInfo(void *p)
{
	REPORT_MFC_DATA *pReportMFC = (REPORT_MFC_DATA *)p;
	HWND window = ::FindWindow(NULL, L"AntiyMonitor-MFC");
	ULONG dwUpdate = -1;
	switch (pReportMFC->ReportType)
	{
		case Report_CreateProcess:
		{
			dwUpdate = UDDM_CREATEPROCESS;
		}
		break;
		case Report_TerminateProc:
		{
			dwUpdate = UDDM_TERMINPROC;
		}
		break;
		case Report_CreateRemoteThread:
		{
			dwUpdate = UDDM_CREATEREMOTETHREAD;
		}
		break;
	}
	::SendMessage(window, WM_UPDATEETWDATA, (WPARAM)p, dwUpdate);
	
}

UINT32 __stdcall WmiMonitorThreadRoutine(void *pContext)
{
	if (!CantiyWmiClass::ShareInstance()->CreateWmiEventMon())
	{
		MessageBox(NULL,_T("WMI���ü�⿪��ʧ��"), _T("��ʾ"), 0);
	}
	return 1;
}

// CAntiyMonitorMFCDlg ��Ϣ�������

BOOL CAntiyMonitorMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	CAntiyResource::ReleasePeResourceFileToSystem(L"sys", L"BIN");

	InitMonitorData();

	CAntiyMiniFilter::ShareInstance()->InitMiniFilter();

	StartEventTrace(1);

	_beginthreadex(NULL,0,WmiMonitorThreadRoutine,NULL,0,NULL);

	CAntiyEventTraceData::ShareInstance()->m_EtwCallback = CAntiyMonitorMFCDlg::CheckData;
	CAntiyMiniFilter::ShareInstance()->m_Callback = CAntiyMonitorMFCDlg::ShowProcessInfo;

	m_TabCtrl.InsertItem(0, _T("ETW׷��"));
	m_TabCtrl.InsertItem(1, _T("WMI���ü��"));
	m_TabCtrl.InsertItem(2, _T("������Ϣ"));
	
	m_WmiView.Create(IDD_WMIDlg, &m_TabCtrl);
	m_EtwView.Create(IDD_EtwDlg, &m_TabCtrl);
	m_ProcessView.Create(IDD_PROCESS,&m_TabCtrl);

	m_pTabItems[0] = &m_EtwView;
	m_pTabItems[1] = &m_WmiView;
	m_pTabItems[2] = &m_ProcessView;
	CRect rc;
	m_TabCtrl.GetClientRect(rc);
	rc.top += 25;
	m_WmiView.MoveWindow(&rc);
	m_EtwView.MoveWindow(&rc);
	m_ProcessView.MoveWindow(&rc);
	m_EtwView.ShowWindow(SW_SHOW);
	m_WmiView.ShowWindow(SW_HIDE);
	m_ProcessView.ShowWindow(SW_HIDE);

	m_EtwView.m_ListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	WCHAR *titles[TITLECOUNT] = { L"Time of Day",L"Process Name",L"PID",L"Operation",L"Path",L"Result",L"Detail" };
	for (ULONG i = 0; i < TITLECOUNT; ++i)
	{
		ULONG ColumnWidth = 150;
		if (i == 4)
		{
			ColumnWidth = 300;
		}
		m_EtwView.m_ListCtrl.InsertColumn(i, titles[i], LVCFMT_LEFT, ColumnWidth);
	}

	m_ProcessView.m_ListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	WCHAR *procTitles[15] = {L"ProcessId",L"UTC Time",L"CommandLine",L"ImagePath",L"CurrentDirectory",L"FileVersion",L"FileDescription",L"ProductName",L"CompanyName",L"ParentCommandLine",L"ParentImagePath",L"ParentDirectory",L"MD5",L"Sha1",L"Sha256"};

	for (ULONG i = 0; i < 15; ++i)
	{
		ULONG ColumnWidth = 200;
		m_ProcessView.m_ListCtrl.InsertColumn(i, procTitles[i], LVCFMT_LEFT, ColumnWidth);
	}

	m_WmiView.HideControlDriverBtn();


	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CAntiyMonitorMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CAntiyMonitorMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CAntiyMonitorMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAntiyMonitorMFCDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	ProcessExitCallbackRoutine(0);

	delete CantiyWmiClass::ShareInstance();

 	CAntiyMiniFilter::ShareInstance()->StopFilter();
 
 	CAntiyMiniFilter::ShareInstance()->UnInstallMiniFilter();

	CDialogEx::OnClose();
}

void CAntiyMonitorMFCDlg::OnTabCtrlSelectChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_CurSelTab = m_TabCtrl.GetCurSel();
	for (ULONG i = 0; i < MAX_TAB_ITEM; ++i)
	{
		if (i == m_CurSelTab)
			m_pTabItems[i]->ShowWindow(SW_SHOW);
		else
			m_pTabItems[i]->ShowWindow(SW_HIDE);
	}

	*pResult = 0;
}

