#include <stdio.h>
#define INITGUID
#include "AntiyEtw.h"

#define ANTIY_TRACE_NAME L"ANTIY TRACE"
#define _AFXDLL
typedef void *HEVENT;

HEVENT g_WaitThreadEvent = NULL;
LARGE_INTEGER g_PerformanceCount;
LARGE_INTEGER g_Frequency;
FILETIME g_SystemFileTime;
HANDLE g_ConsumerThreadHandle = 0;
BOOL g_bIsStartTrace = FALSE;

UINT StartEventTrace(ULONG IsStart);

UINT WINAPI ProcessTimeThread(void *lpContext)
{
	LARGE_INTEGER PerformanceCount = { 0x00 };
	while (WaitForSingleObject(g_WaitThreadEvent, 250) == STATUS_TIMEOUT)
	{
		//LockEtw();

		QueryPerformanceCounter(&PerformanceCount);
		ULONG LowPart = PerformanceCount.LowPart - g_PerformanceCount.LowPart;
		LONG HighPart = PerformanceCount.HighPart - g_PerformanceCount.HighPart;
		ULONGLONG QuadPart = PerformanceCount.QuadPart - g_PerformanceCount.QuadPart;

		LARGE_INTEGER lg1 = { 0x00 };
		LARGE_INTEGER lg2 = { 0x00 };
		LARGE_INTEGER ft = { 0x00 };

		if (HighPart >= 0)
		{
			lg1.QuadPart = (10000000 * (QuadPart % g_Frequency.QuadPart)) / g_Frequency.QuadPart + (10000000 * (QuadPart / g_Frequency.QuadPart));
			ft.LowPart = lg1.LowPart;

		}
		else
		{
			lg1.QuadPart = CalcByTraceTimeLessPerformanceCount(&g_SystemFileTime, -HighPart);
			ft.LowPart = lg1.LowPart;
		}
		ft.HighPart = g_SystemFileTime.dwHighDateTime;
		lg2.LowPart += g_SystemFileTime.dwLowDateTime;
		lg1.LowPart = g_SystemFileTime.dwLowDateTime;

		ft.HighPart = (ft.QuadPart + lg2.QuadPart) >> 32;

		//UnlockEtw();
	}
	return 0;
}

ULONG GetSystemVersion(VOID)
{
#pragma warning(disable:4996)
	return GetVersion();
}

ULONG CompVersionEx(void)
{
	ULONG dwResult = 0;
	OSVERSIONINFOEX os_info = { 0x00 };
	os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	os_info.dwMajorVersion = 0;
	memset(&os_info.dwMinorVersion, 0x00, sizeof(OSVERSIONINFOW));

	GetVersionEx((OSVERSIONINFO *)&os_info);
	if (os_info.dwMajorVersion >= 6)
		return 1;
	if (os_info.dwMajorVersion != 5 || os_info.dwMinorVersion == 0)
		return 0;
	if (os_info.dwMajorVersion == 2)
		return (os_info.wProductType == VER_NT_WORKSTATION);
	if (os_info.dwMinorVersion == 1)
		return os_info.wServicePackMajor >= 2;
	else
		return 0;
	
}



ULONG CompVista(VOID)
{
	ULONG dwResult = 0;
	if (LOBYTE(GetVersion()) >= 6)
	{
		if (HIBYTE(GetVersion()) >= 2)
			dwResult = 1;
	}
	return dwResult;

}

UINT WINAPI ConsumeThreadRoutine(void *lpContext)
{
	EVENT_TRACE_LOGFILE logfile = { 0x00 };
	ULONG dwResult = 0;
	LPWSTR lpwTraceName = (LPWSTR)KERNEL_LOGGER_NAME;
	TRACEHANDLE hTraceHanle;
	BOOL isStop = FALSE;

	if (CompVista())
	{
		lpwTraceName = (LPWSTR)ANTIY_TRACE_NAME;
	}

	ULONG dwVersion = 0;
	logfile.LoggerName = lpwTraceName;//指定事件名称，该域和logfilename互斥，摘自msdn
	logfile.BufferCallback = EventTraceLogfileBufferCallback;
	//logfile.EventRecordCallback = (PEVENT_RECORD_CALLBACK)EventTraceLogfileEventRecordCallback;
	logfile.EventCallback = EventTraceLogfileEventCallback;
	logfile.CurrentTime = 0;
	logfile.BuffersRead = 0;
	logfile.Context = 0;
	logfile.LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_ADD_HEADER_MODE;
	//开启跟踪回话，并返回回话句柄
	hTraceHanle = CAntiyEventTraceData::ShareInstance()->m_OpenTraceFunc(&logfile);
	if (!hTraceHanle)   
	{
		printf("OpenTraceW faild!\n");
		return -1;
	}

	UINT dwThreadId = 0;
	HANDLE hUpdateTimeThread = (HANDLE)_beginthreadex(0, 0, ProcessTimeThread, 0, 0, &dwThreadId);

	dwResult = CAntiyEventTraceData::ShareInstance()->m_ProcessTraceFunc(&hTraceHanle, 1, 0, 0);

	if (WaitForSingleObject(g_WaitThreadEvent, 0) == STATUS_TIMEOUT)
	{
		isStop = TRUE;
		SetEvent(g_WaitThreadEvent);
	}
	
	WaitForSingleObject(hUpdateTimeThread, INFINITE);
	CloseHandle(hUpdateTimeThread);
	if (isStop)
	{
		CloseHandle(g_ConsumerThreadHandle);
		g_ConsumerThreadHandle = 0;
		ResetEvent(g_WaitThreadEvent);
		g_bIsStartTrace = 0;

		StartEventTrace(1);
	}

	return 0;
}


BOOL
WINAPI ProcessExitCallbackRoutine
(
	_In_ DWORD CtrlType
	)
{
	StartEventTrace(0);
	return TRUE;
}

void InitMonitorData(void)
{
	if (!g_WaitThreadEvent)
	{
		g_WaitThreadEvent = CreateEvent(0, 1, 0, 0);
	}
	InitTimeData();
}

UINT StartEventTrace(ULONG IsStart)
{
	UINT dwResult = 0;
	PEVENT_TRACE_PROPERTIES pEventTracePropertie;
	TRACEHANDLE hSessionHandle = 0;
	TRACEHANDLE hr = 0;
	HANDLE hConsumeThread = 0;
	UINT dwConsumeThreadId = 0;
	/*DWORD dwVersion = 0;*/
	//event trace name
	LPCWSTR lpwTraceName = KERNEL_LOGGER_NAME;
	ULONG dwBufferSize = sizeof(EVENT_TRACE_PROPERTIES) + 2 * MAX_PATH * sizeof(WCHAR) + sizeof(ULONG) * 10;
	
	PVOID lpProviderId = NULL;
	HMODULE hAdvModule = GetModuleHandle(L"advapi32.dll");
	CAntiyEventTraceData::ShareInstance()->m_StartTraceFunc = (STARTTRACEW)GetProcAddress(hAdvModule, "StartTraceW");
	CAntiyEventTraceData::ShareInstance()->m_ControlTraceFunc = (CONTROLTRACEW)GetProcAddress(hAdvModule, "ControlTraceW");
	CAntiyEventTraceData::ShareInstance()->m_OpenTraceFunc = (OPENTRACEW)GetProcAddress(hAdvModule, "OpenTraceW");
	CAntiyEventTraceData::ShareInstance()->m_ProcessTraceFunc = (PROCESSTRACE)GetProcAddress(hAdvModule, "ProcessTrace");

	pEventTracePropertie = (EVENT_TRACE_PROPERTIES *)malloc(dwBufferSize);
	if (!pEventTracePropertie)
	{
		printf("alloc lpEventConfig faild!\n");
		return 0;
	}

	if (g_bIsStartTrace == IsStart)
		return 0;

	if (CompVista())
	{
		lpwTraceName = ANTIY_TRACE_NAME;
	}

	memset(pEventTracePropertie, 0x00, dwBufferSize);
	pEventTracePropertie->Wnode.BufferSize = dwBufferSize;
	pEventTracePropertie->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
	pEventTracePropertie->Wnode.ClientContext = 1;
	pEventTracePropertie->FlushTimer = 1;
	pEventTracePropertie->LogFileMode = 0;
	if (CompVista())
	{
		pEventTracePropertie->LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_SYSTEM_LOGGER_MODE;
	}
	else
	{
		pEventTracePropertie->LogFileMode = EVENT_TRACE_REAL_TIME_MODE | 0;
		pEventTracePropertie->Wnode.Guid = SystemTraceControlGuid;
		
	}
	
	pEventTracePropertie->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
	pEventTracePropertie->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(WCHAR) * MAX_PATH;
	StringCchCopy((LPWSTR)((char *)pEventTracePropertie + pEventTracePropertie->LoggerNameOffset), MAX_PATH, lpwTraceName);

	if (CompVersionEx())
	{
		pEventTracePropertie->EnableFlags = EVENT_TRACE_FLAG_EXTENSION;
		pEventTracePropertie->EnableFlags |= 0x00ff0000;
		pEventTracePropertie->EnableFlags |= (EVENT_TRACE_FLAG_DISK_IO_INIT | EVENT_TRACE_FLAG_SYSTEMCALL | EVENT_TRACE_FLAG_PROCESS_COUNTERS);
		*((ULONG *)((char *)pEventTracePropertie + pEventTracePropertie->LogFileNameOffset + sizeof(WCHAR) * MAX_PATH)) = 0x1000A;
		*((ULONG *)((char *)pEventTracePropertie + pEventTracePropertie->LogFileNameOffset + sizeof(WCHAR) * MAX_PATH + 4)) = 0x10009;
		*((ULONG *)((char *)pEventTracePropertie + pEventTracePropertie->LogFileNameOffset + sizeof(WCHAR) * MAX_PATH + 8)) = 0x10000;

	}
	else
		pEventTracePropertie->EnableFlags |= EVENT_TRACE_FLAG_NETWORK_TCPIP;

	if (IsStart)
	{
		dwResult = CAntiyEventTraceData::ShareInstance()->m_StartTraceFunc(&hSessionHandle, lpwTraceName, pEventTracePropertie);
		if (dwResult && dwResult != ERROR_ALREADY_EXISTS)
		{
			MessageBox(0,L"开启跟踪回话失败",L"提示",0);
		}
		else
		{
			g_ConsumerThreadHandle = (HANDLE)_beginthreadex(0, 0, ConsumeThreadRoutine, 0, 0, &dwConsumeThreadId);
			g_bIsStartTrace = IsStart;
			dwResult = 0;
		}
	}
	else
	{
		dwResult = CAntiyEventTraceData::ShareInstance()->m_ControlTraceFunc(hSessionHandle, lpwTraceName, pEventTracePropertie, EVENT_TRACE_CONTROL_STOP);
		if (dwResult == S_OK)
			g_bIsStartTrace = 0;
	}


	return dwResult;
}

