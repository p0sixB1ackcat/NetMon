#include "AntiyMiniFilter.h"
#include "AntiyMonUK.h"
#include "Windows.h"
#include <fltUser.h>
#include <assert.h>
#include "AntiyMonReport.h"
#include <sddl.h>

#pragma pack(1)
typedef struct _AntiyMFilter_MESSAGE
{
	FILTER_MESSAGE_HEADER MessageHeader;
	ANTIYMFILTER_REPORT_R3DATA ReceiveKernelData;
	OVERLAPPED Ovlp;

}AntiyMFilter_MESSAGE, *PAntiyMFilter_MESSAGE;

typedef struct _AntiyMFilter_REPLY_MESSAGE {

	FILTER_REPLY_HEADER ReplyHeader;

	AntiyMFilter_REPLY Reply;

} AntiyMFilter_REPLY_MESSAGE, *PAntiyMFilter_REPLY_MESSAGE;

CAntiyMiniFilter *CAntiyMiniFilter::m_ShareInstance = NULL;
CLock CAntiyMiniFilter::m_Clock = CLock();

CAntiyMiniFilter *CAntiyMiniFilter::ShareInstance(void)
{
	if (!m_ShareInstance)
	{
		CAutoLock autoLock(&m_Clock);
		if (!m_ShareInstance)
			m_ShareInstance = new CAntiyMiniFilter();
	}
	return m_ShareInstance;

}

BOOL CAntiyMiniFilter::InitMiniFilter(void)
{
	if (!InstallMiniFilter())
	{
		MessageBox(0,L"安装驱动失败",L"提示",0);
		return FALSE;
	}

	if (!StartFilter())
	{
		MessageBox(0,L"开启驱动失败",L"提示",0);
		return FALSE;
	}
	if (!CreatePort(ATPortName))
	{
		MessageBox(0,L"开启端口失败",L"提示",0);
		return FALSE;
	}


	return TRUE;
}

BOOL CAntiyMiniFilter::InstallMiniFilter(void)
{
	//SCM管理器句柄
	SC_HANDLE hServicesMgr = NULL;
	//NT驱动程序的句柄
	SC_HANDLE hNt = NULL;
	TCHAR DriverFullPathNameBuffer[MAX_PATH] = {0x00};
	TCHAR FormatStr[MAX_PATH];
	TCHAR *RegPath = L"SYSTEM\\CurrentControlSet\\Services\\";
	HKEY hKey = 0;
	DWORD dwData = 0;

	hServicesMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hServicesMgr)
	{
		CloseServiceHandle(hServicesMgr);
		return FALSE;
	}

	ExpandEnvironmentStrings(L"%systemroot%", DriverFullPathNameBuffer,sizeof(DriverFullPathNameBuffer));
	wcsncat(DriverFullPathNameBuffer,L"\\",1);
	wcsncat(DriverFullPathNameBuffer,DRIVER_NAME,wcslen(DRIVER_NAME));
	wcsncat(DriverFullPathNameBuffer,L".sys",wcslen(L".sys"));

	//GetFullPathName(pSysPath, MAX_PATH, DriverFullPathNameBuffer, NULL);

	hNt = CreateService(hServicesMgr
		, DRIVER_NAME
		, DRIVER_NAME
		, SERVICE_ALL_ACCESS
		, SERVICE_FILE_SYSTEM_DRIVER
		, SERVICE_DEMAND_START
		, SERVICE_ERROR_IGNORE
		, DriverFullPathNameBuffer
		, L"FSFilter Activity Monitor"
		, NULL
		, L"FltMgr"
		, NULL
		, NULL);

	if (!hNt)
	{
		ULONG error = GetLastError();
		if (error == ERROR_SERVICE_EXISTS || error == 0x00000431)
		{
			CloseServiceHandle(hServicesMgr);
			CloseServiceHandle(hNt);
			return TRUE;
		}
		else
		{
			CloseServiceHandle(hServicesMgr);
			CloseServiceHandle(hNt);
			return FALSE;
		}
	}

	CloseServiceHandle(hServicesMgr);
	CloseServiceHandle(hNt);

	wcsncpy_s(FormatStr, RegPath, wcslen(RegPath));
	wcsncat_s(FormatStr, DRIVER_NAME, sizeof(DRIVER_NAME));
	wcsncat_s(FormatStr, L"\\Instances", wcslen(L"\\Instances"));

	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE
		, FormatStr
		, 0
		, L""
		, REG_OPTION_NON_VOLATILE
		, KEY_ALL_ACCESS
		, NULL
		, &hKey
		, &dwData) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	wcsncpy_s(FormatStr, DRIVER_NAME, wcslen(DRIVER_NAME));
	wcsncat_s(FormatStr,L" Instance", wcslen(L" Instance"));
	if (RegSetValueEx(hKey
		, L"DefaultInstance"
		, 0
		, REG_SZ
		, (const BYTE *)FormatStr
		, wcslen(FormatStr) * sizeof(WCHAR)) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	RegFlushKey(hKey);
	RegCloseKey(hKey);

	//SYSTEM_LOCAL_MACHINE\CurrentControlSet\Services\DriverName\Instances\DriverName Instance
	wcsncpy_s(FormatStr, RegPath, wcslen(RegPath));
	wcsncat_s(FormatStr, DRIVER_NAME, wcslen(DRIVER_NAME));
	wcsncat_s(FormatStr, L"\\Instances\\", wcslen(L"\\Instances\\"));
	wcsncat_s(FormatStr, DRIVER_NAME, wcslen(DRIVER_NAME));
	wcsncat_s(FormatStr, L" Instance", wcslen(L" Instance"));

	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE
		, FormatStr
		, 0
		, L""
		, REG_OPTION_NON_VOLATILE
		, KEY_ALL_ACCESS
		, NULL
		, &hKey
		, &dwData) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	wcsncpy_s(FormatStr, DRIVER_ALTITUDE, wcslen(DRIVER_ALTITUDE));
	if (RegSetValueEx(hKey
		, L"Altitude"
		, 0
		, REG_SZ
		, (const BYTE *)FormatStr
		, (DWORD)(wcslen(FormatStr) * sizeof(WCHAR))) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	dwData = 0;
	if (RegSetValueEx(hKey
		, L"Flags"
		, 0
		, REG_DWORD
		, (const BYTE *)&dwData
		, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	RegFlushKey(hKey);
	RegCloseKey(hKey);

	return TRUE;
}

BOOL CAntiyMiniFilter::UnInstallMiniFilter(void)
{
	SC_HANDLE schManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS svcStatus;

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!schManager)
	{
		return FALSE;
	}

	schService = OpenService(schManager,DRIVER_NAME,SERVICE_ALL_ACCESS);
	if (!schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}
	
	ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus);
	if (!DeleteService(schService))
	{
		CloseServiceHandle(schManager);
		CloseServiceHandle(schService);
		return FALSE;
	}

	CloseServiceHandle(schManager);
	CloseServiceHandle(schService);
	return TRUE;

}

BOOL CAntiyMiniFilter::StartFilter(void)
{
	SC_HANDLE schManager = NULL;
	SC_HANDLE schService = NULL;
	DWORD errorCode = 0;

	schManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (!schManager)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}

	schService = OpenService(schManager,DRIVER_NAME,SERVICE_ALL_ACCESS);
	if (!schService)
	{
		CloseServiceHandle(schManager);
		CloseServiceHandle(schService);
		return FALSE;
	}

	if (!StartService(schService, 0, NULL))
	{
		errorCode = GetLastError();
		CloseServiceHandle(schManager);
		CloseServiceHandle(schService);
		if (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)
		{
			return TRUE;
		}
		
		return FALSE;
	}

	CloseServiceHandle(schManager);
	CloseServiceHandle(schService);

	return TRUE;
}

BOOL CAntiyMiniFilter::StopFilter(void)
{
	SC_HANDLE schManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS svcStatus;
	
	schManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (!schManager)
	{
		return FALSE;
	}

	schService = OpenService(schManager,DRIVER_NAME,SERVICE_ALL_ACCESS);
	if (!schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}

	if (!ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus) && svcStatus.dwCurrentState != SERVICE_STOPPED)
	{
		CloseServiceHandle(schManager);
		CloseServiceHandle(schService);
		return FALSE;
	}

	CloseServiceHandle(schManager);
	CloseServiceHandle(schService);
	return TRUE;
}

BOOL CAntiyMiniFilter::CreatePort(PCWSTR PortName)
{
	HANDLE threadHandle = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)CAntiyMiniFilter::ShareInstance()->AntiyMFilterWait,(LPVOID)PortName,0,NULL);
	if (!threadHandle)
		return FALSE;

	return TRUE;
}

DWORD CAntiyMiniFilter::AntiyMFilterWait(PVOID lpContext)
{
	BOOL bResult = TRUE;
	AntiyMFilter_Thread_Context Context;
	HANDLE hPort = 0;
	HANDLE hCompletion = 0;
	HRESULT hr = 0;
	DWORD dwRet = 0;
	HANDLE ThreadHandles[2] = { 0 };
	ULONG i, j;
	DWORD threadId = 0;
	AntiyMFilter_MESSAGE *msg = NULL;
	WCHAR *szPortName = (WCHAR *)lpContext;
	if (!szPortName)
	{
		szPortName = ATPortName;
	}

	hr = FilterConnectCommunicationPort(szPortName, 0, NULL, 0, NULL, &hPort);
	if (IS_ERROR(hr))
	{
		bResult = FALSE;
		goto ret;
	}
	hCompletion = CreateIoCompletionPort(hPort, NULL, 0, CAntiyMiniFilter::ShareInstance()->m_DefaultThreadCount);
	if (!hCompletion)
	{
		bResult = FALSE;
		goto ret;
	}

	FilterSendMessage(hPort, L"Initializer R0 and R3 Connect!", wcslen(L"Initializer R0 and R3 Connect!"), NULL, 0, &dwRet);
	Context.hPort = hPort;
	Context.hCompletion = hCompletion;

	for (i = 0; i < CAntiyMiniFilter::ShareInstance()->m_DefaultThreadCount; ++i)
	{
		ThreadHandles[i] = CreateThread(NULL
			, 0
			, (LPTHREAD_START_ROUTINE)CAntiyMiniFilter::ShareInstance()->AntiyMFilterWork
			, &Context
			, 0
			, &threadId);

		if (!ThreadHandles[i])
		{
			bResult = FALSE;
			goto ret;
		}

		for (j = 0; j < CAntiyMiniFilter::ShareInstance()->m_DefaultRequestCount; ++j)
		{
			msg = (PAntiyMFilter_MESSAGE)malloc(sizeof(AntiyMFilter_MESSAGE));
			if (!msg)
			{
				goto ret;
			}

			memset(msg, 0x00, sizeof(AntiyMFilter_MESSAGE));
			hr = FilterGetMessage(hPort, &msg->MessageHeader, FIELD_OFFSET(AntiyMFilter_MESSAGE, Ovlp), &msg->Ovlp);
			if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
			{
				free(msg);
				msg = NULL;
				goto ret;
			}
		}
	}

	bResult = TRUE;

	WaitForMultipleObjectsEx(i, ThreadHandles, TRUE, INFINITE, FALSE);

ret:
	if (hPort)
		CloseHandle(hPort);
	if (hCompletion)
		CloseHandle(hCompletion);
	return bResult;
}

DWORD CAntiyMiniFilter::AntiyMFilterWork(AntiyMFilter_Thread_Context *Context)
{
	BOOL bResult = FALSE;
	DWORD dwOutSize = 0;
	ULONG_PTR uKey = 0;
	LPOVERLAPPED pOvlp = NULL;
	AntiyMFilter_MESSAGE *message = NULL;
	HRESULT hr = 0;
	ANTIYMFILTER_PROCESSINFO *pProcInfo = NULL;
	AntiyMFilter_REPLY_MESSAGE replyMessage;
	HANDLE hToken = 0;
	HANDLE hProcess = 0;
	ANTIYMFILTER_EVENT_TYPE KernelReportType;
	ANTIYMFILTER_TERMINATE_PROCESSINFO *pTerProcInfo = NULL;

	while (1)
	{
		bResult = GetQueuedCompletionStatus(Context->hCompletion, &dwOutSize, &uKey, &pOvlp, INFINITE);
		message = CONTAINING_RECORD(pOvlp,AntiyMFilter_MESSAGE,Ovlp);
		if (!bResult)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			bResult = FALSE;
			break;
		}

		KernelReportType = message->ReceiveKernelData.Type;
		pProcInfo = &message->ReceiveKernelData.ReportData.ProcInfo;

		assert(pProcInfo->ImagePath != NULL);
		__analysis_assume(pProcInfo->ImagePath != NULL);

		replyMessage.ReplyHeader.Status = 0;
		replyMessage.ReplyHeader.MessageId = message->MessageHeader.MessageId;
		replyMessage.Reply.SafeToOpen = TRUE;

		switch (KernelReportType)
		{
			case ATMFilter_CreateProcess:
			{
				REPORT_MFC_DATA *pReportData = NULL;

				pReportData = (REPORT_MFC_DATA *)malloc(sizeof(REPORT_MFC_DATA));
				memset(pReportData, 0x00, sizeof(REPORT_MFC_DATA));
				CopyMemory(pReportData->CommandLine, pProcInfo->CommandLine, sizeof(pReportData->CommandLine));
				CopyMemory(pReportData->CurrentDirtory, pProcInfo->CurrentDirtory, sizeof(pReportData->CurrentDirtory));
				CopyMemory(pReportData->ImagePath, pProcInfo->ImagePath, sizeof(pReportData->ImagePath));
				CopyMemory(pReportData->ParentCommandLine, pProcInfo->ParentCommandLine, sizeof(pReportData->ParentCommandLine));
				CopyMemory(pReportData->ParentImagePath, pProcInfo->ParentImagePath, sizeof(pReportData->ParentImagePath));
				CopyMemory(pReportData->ParentCurrentDirtyory, pProcInfo->ParentCurrentDirtyory, sizeof(pReportData->ParentCurrentDirtyory));
				pReportData->ReportType = Report_CreateProcess;
				pReportData->ProcessId = pProcInfo->ProcessId;
				pReportData->TerminalSessionId = pProcInfo->TerminalSessionId;
				pReportData->UserTokenSid = pProcInfo->UserTokenSid;
				pReportData->LogonId = pProcInfo->LogonId;
				pReportData->TimeStamp = pProcInfo->TimeStamp;
				ATGetFileAllInfo(pProcInfo->ImagePath, &pReportData->Local_File_Info);

				if (CAntiyMiniFilter::ShareInstance()->m_Callback)
				{
					CAntiyMiniFilter::ShareInstance()->m_Callback((void *)pReportData);
				}
				free(pReportData);
				pReportData = NULL;
			}
			break;
			case ATMFilter_CloseProcess:
			{
				pTerProcInfo = &message->ReceiveKernelData.ReportData.TerMinateProcInfo;
				if (CAntiyMiniFilter::ShareInstance()->m_Callback)
				{
					CAntiyMiniFilter::ShareInstance()->m_Callback(pTerProcInfo);
				}
				
			}
			break;
		}


		hr = FilterReplyMessage(Context->hPort, (PFILTER_REPLY_HEADER)&replyMessage, sizeof(AntiyMFilter_REPLY_MESSAGE));


		if (SUCCEEDED(hr))
		{

		}
		else
		{
			bResult = FALSE;
			break;
		}
		
		memset(&message->Ovlp, 0x00, sizeof(OVERLAPPED));
 		hr = FilterGetMessage(Context->hPort, &message->MessageHeader, FIELD_OFFSET(AntiyMFilter_MESSAGE, Ovlp), &message->Ovlp);
 		if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
 		{
			bResult = FALSE;
 			break;
 		}
	}

	free(message);
	return bResult;
}

CAntiyMiniFilter::CAntiyMiniFilter()
{
	m_DefaultThreadCount = 2;
	m_DefaultRequestCount = 5;
	m_MaxThreadCount = 64;
}


CAntiyMiniFilter::~CAntiyMiniFilter()
{
}
