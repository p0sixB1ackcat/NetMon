#include <WbemCli.h>
#include <atlcomcli.h>
#include <comutil.h>
#include <evntrace.h>
#include <evntcons.h>
#include <process.h>
#include <strsafe.h>
#include <versionhelpers.h>
#include <tdh.h>
#include "AntiyWmi.h"
#include "AntiyEtw.h"

#define LOGGERFILENAME L"Wmi"

#pragma comment(lib, "ws2_32.lib") 
#pragma  comment(lib,"Wbemuuid.lib")
#pragma  comment(lib,"comsuppw.lib")
#pragma  comment(lib,"Tdh.lib")
#pragma comment(lib,"Advapi32.lib")
#pragma comment(lib,"Psapi.lib")


DEFINE_GUID(clsdUnsecurtedAdattor, 
0x49BD2028, 0x1523, 0x11D1, 0x0AD, 0x79, 0x00, 0x0C0, 0x4F, 0x0D8, 0x0FD, 0xFF);


DEFINE_GUID(IID_UnsecureAdaptor, 
0x1CFABA8C, 0x1523, 0x11D1, 0xAD, 0x79, 0, 0xC0, 0x4F, 0xD8, 0xFD, 0xFF);

CantiyWmiClass *CantiyWmiClass::m_Instance = NULL;

CantiyWmiClass *CantiyWmiClass::ShareInstance()
{
	if (!m_Instance)
	{
		m_Instance = new CantiyWmiClass();
	}
	return m_Instance;
}

CantiyWmiClass::CantiyWmiClass()
{
	InitListEntry(&m_EtwWmiDataListHeader);
	InitializeCriticalSection(&m_EtwWmiDataLock);
	m_pReport = NULL;
}
CantiyWmiClass::~CantiyWmiClass()
{
	CloseTraceAndSession();
}

BOOL iSShowEventTraceData(USHORT EventId)
{
	BOOL bResult = FALSE;
	static DWORD isWindows8Later = -1;
	if (isWindows8Later == -1)
	{
		isWindows8Later = IsWindows8OrGreater() ? 1 : 0;
	}

	do
	{
		if (isWindows8Later)
		{
			if (EventId == 11 || EventId == 13)
				bResult = TRUE;
			else
				bResult = FALSE;
			break;
		}
		if (EventId == 1 || EventId == 3)
			bResult = TRUE;
		else
			bResult = FALSE;

	} while (0);

	return bResult;
}

void CantiyWmiClass::FreeEtwWmiData(void)
{
	LIST_ENTRY *pList = NULL;
	LIST_ENTRY *pFreeList = NULL;
	CEtwWmiData *pEtwWmiData = NULL;
	EnterCriticalSection(&m_EtwWmiDataLock);
	do
	{
		if (IsEmptyList(&m_EtwWmiDataListHeader))
			break;

		pList = m_EtwWmiDataListHeader.Flink;
		while (pList != &m_EtwWmiDataListHeader)
		{
			pEtwWmiData = CONTAINING_RECORD(pList, CEtwWmiData, m_List);
			if (pEtwWmiData)
			{
				pFreeList = pList;
				pList = pList->Flink;
				RemoveHeaderList(pFreeList);
				delete pEtwWmiData;
			}
			else
			{
				pList = pList->Flink;
			}

		}

	} while (0);


	LeaveCriticalSection(&m_EtwWmiDataLock);
}

void CantiyWmiClass::ReportWmiData(REPORT_WMI_EVENT *pReportWmiData)
{
	ULONG i;
	if (!pReportWmiData)
		return;

	if (!m_pReport)
	{
		m_pReport = (PREPORT_WMI_EVENT)malloc(sizeof(REPORT_WMI_EVENT));
	}

	CopyMemory(m_pReport, pReportWmiData, sizeof(REPORT_WMI_EVENT));
	UpdateData();

}

UINT32 CantiyWmiClass::WaitReceiveWmiThreadRoutine(void *pContext)
{
	LIST_ENTRY *pList = NULL;
	LIST_ENTRY *pReleaseList = NULL;
	CEtwWmiData *pEtwWmiData = NULL;
	WMI_EVENT_DATA *pWmiEventData = NULL;
	REPORT_WMI_EVENT *pReportData = NULL;
	WCHAR szBuffer[0x30] = { 0x00 };
	ULONG dwCurOperaIndex = 0;

	//如果Wmi的3类事件没有发生，则不做任何操作，并且清空ETW跟踪Wmi事件链表
	if (IsEmptyList(&CantiyWmiClass::ShareInstance()->m_WmiListernSink->m_ListHeader))
	{
		CantiyWmiClass::ShareInstance()->FreeEtwWmiData();
		return 1;
	}

	pList = CantiyWmiClass::ShareInstance()->m_WmiListernSink->m_ListHeader.Flink;
	//while (pList != &g_WmiListernSink->m_ListHeader)
	{
		pWmiEventData = CONTAINING_RECORD(pList, WMI_EVENT_DATA, List);

	}

	if (!pWmiEventData)
	{
		return -1;
	}

	pReportData = (PREPORT_WMI_EVENT)malloc(sizeof(REPORT_WMI_EVENT));
	memset(pReportData, 0, sizeof(REPORT_WMI_EVENT));
	pReportData->szClassName = pWmiEventData->szClassName->Buffer;
	pReportData->szServerName = pWmiEventData->szServerName->Buffer;
	pReportData->szSuperClass = pWmiEventData->szSuperClassName->Buffer;
	pReportData->szQuery = pWmiEventData->szQuery ? pWmiEventData->szQuery->Buffer : NULL;
	pReportData->szQueryLanguage = pWmiEventData->szQueryLanguage ? pWmiEventData->szQueryLanguage->Buffer : NULL;
	pReportData->szName = pWmiEventData->szName ? pWmiEventData->szName->Buffer : NULL;
	pReportData->szNamespace = pWmiEventData->szNameSpace ? pWmiEventData->szNameSpace->Buffer : NULL;
	pReportData->szEventName = pWmiEventData->szEventName ? pWmiEventData->szEventName->Buffer : NULL;
	FILETIME ft, localFt;
	SYSTEMTIME st;
	ft.dwLowDateTime = pWmiEventData->llTimeCreate.LowPart;
	ft.dwHighDateTime = pWmiEventData->llTimeCreate.HighPart;
	FileTimeToLocalFileTime(&ft, &localFt);
	FileTimeToSystemTime(&localFt, &st);

	wsprintf(szBuffer, L"%04d-%02d-%02d %02d:%02d:%02d.%d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	pReportData->szTime = szBuffer;

	EnterCriticalSection(&CantiyWmiClass::ShareInstance()->m_EtwWmiDataLock);
	pList = CantiyWmiClass::ShareInstance()->m_EtwWmiDataListHeader.Flink;
	while (pList && pList != &CantiyWmiClass::ShareInstance()->m_EtwWmiDataListHeader)
	{
		do
		{
			pEtwWmiData = CONTAINING_RECORD(pList, CEtwWmiData, m_List);
			if (!pEtwWmiData)
				break;

			if (pEtwWmiData->m_ClientProcessid)
				pReportData->dwProcessId = pEtwWmiData->m_ClientProcessid;
			if (!pReportData->szProcessFullPath)
				pReportData->szProcessFullPath = pEtwWmiData->m_ProcessFullPath;

			if (pReportData->OperaCount < MAX_PATH)
			{
				pReportData->OperaList[pReportData->OperaCount] = pEtwWmiData->m_Operation;

				pReportData->OperaCount++;
			}

		} while (0);

		pList = pList->Flink;
	}

	CantiyWmiClass::ShareInstance()->ReportWmiData(pReportData);

	while (pList && pList != &CantiyWmiClass::ShareInstance()->m_EtwWmiDataListHeader)
	{
		pEtwWmiData = CONTAINING_RECORD(pList, CEtwWmiData, m_List);
		if (!pEtwWmiData)
		{
			pList = pList->Flink;
			continue;
		}
		else
		{
			pReleaseList = pList;
			pList = pList->Flink;
			RemoveHeaderList(pReleaseList);
			delete pEtwWmiData;
		}

	}

	LeaveCriticalSection(&CantiyWmiClass::ShareInstance()->m_EtwWmiDataLock);

	CantiyWmiClass::ShareInstance()->m_WmiListernSink->FreeWmiData();

	return 1;
}

VOID WINAPI CantiyWmiClass::EtwEventTraceRecordCallback(PEVENT_RECORD EventRecord)
{
	USHORT bEventId = EventRecord->EventHeader.EventDescriptor.Id;
	ULONG Status = ERROR_SUCCESS;
	TRACE_EVENT_INFO *pEventInfo = NULL;
	ULONG dwEventInfoLen = 0;
	ULONG i;
	LPWSTR szPropertyName = NULL;
	PBYTE pbBuffer = NULL;
	ULONG cbBuffer = 0;


	if (!iSShowEventTraceData(bEventId))
		return;
	Status = TdhGetEventInformation(EventRecord, 0, NULL, NULL, &dwEventInfoLen);


	CEtwWmiData *pEtwWmiData = new CEtwWmiData;
	pEtwWmiData->m_TimeStamp = EventRecord->EventHeader.TimeStamp;

	do
	{
		if (Status == ERROR_INSUFFICIENT_BUFFER)
		{
			pEventInfo = (TRACE_EVENT_INFO *)malloc(dwEventInfoLen);
			if (!pEventInfo)
			{
				printf("Allocate EventInfo Buffer Fail!\n");
				break;
			}

			Status = TdhGetEventInformation(EventRecord, 0, NULL, pEventInfo, &dwEventInfoLen);
			if (Status != ERROR_SUCCESS)
			{
				printf("Get Event Information Fail!\n");
				break;
			}

			if (pEventInfo->DecodingSource != DecodingSourceXMLFile)
				break;

			for (i = 0; i < pEventInfo->PropertyCount; ++i)
			{
				EVENT_PROPERTY_INFO PropertyInfo = pEventInfo->EventPropertyInfoArray[i];
				szPropertyName = (LPWSTR)((PUCHAR)pEventInfo + PropertyInfo.NameOffset);
				PROPERTY_DATA_DESCRIPTOR PropertyDesc = { (ULONGLONG)szPropertyName,0xffffffff,0 };

				if ((PropertyInfo.Flags & PROPERTY_FLAGS::PropertyStruct) == 0)
				{
					switch (PropertyInfo.nonStructType.InType)
					{
					case TDH_INTYPE_UNICODESTRING:
					{
						Status = TdhGetPropertySize(EventRecord, 0, NULL, 1, &PropertyDesc, &cbBuffer);
						if (Status != ERROR_SUCCESS)
						{
							printf("Get PropertySize Fail:%d", GetLastError());
							break;
						}

						pbBuffer = (PBYTE)malloc(cbBuffer + 2);
						if (!pbBuffer)
							break;

						memset(pbBuffer, 0x00, cbBuffer + 2);
						Status = TdhGetProperty(EventRecord, 0, NULL, 1, &PropertyDesc, cbBuffer, pbBuffer);
						if (Status != ERROR_SUCCESS)
						{
							printf("Get Property Fail:%d!\n", GetLastError());
							break;
						}
						pEtwWmiData->Add(szPropertyName, (WCHAR *)pbBuffer);

					}
					break;
					case TDH_INTYPE_HEXINT32:
					case TDH_INTYPE_UINT32:
					{
						DWORD dwBuffer = 0;
						Status = TdhGetProperty(EventRecord, 0, NULL, 1, &PropertyDesc, sizeof(DWORD), (PBYTE)&dwBuffer);
						if (Status != ERROR_SUCCESS)
						{
							printf("Get Property Fail:%d!\n", GetLastError());
							break;
						}

						pEtwWmiData->Add(szPropertyName, (UINT32)dwBuffer);
					}
					break;
					}
				}

			}
		}

	} while (0);

	
	EnterCriticalSection(&CantiyWmiClass::ShareInstance()->m_EtwWmiDataLock);

	InsertHeaderList(&CantiyWmiClass::ShareInstance()->m_EtwWmiDataListHeader, &pEtwWmiData->m_List);

	LeaveCriticalSection(&CantiyWmiClass::ShareInstance()->m_EtwWmiDataLock);

	if (pEventInfo)
		free(pEventInfo);
	if (pbBuffer)
		free(pbBuffer);
	return;

}

ULONG WINAPI CantiyWmiClass::EventBufferCallbackRoutine(PEVENT_TRACE_LOGFILEW Logfile)
{
	WaitReceiveWmiThreadRoutine(NULL);
	return 1;
}

void CantiyWmiClass::CreateGuidStr(WCHAR *szBuffer, size_t dwBufferLen)
{
	HRESULT hr;
	LPOLESTR guidStr = NULL;

	if (!szBuffer)
		return;
	hr = CoCreateGuid(&m_Extension.guid);
	if (SUCCEEDED(hr))
	{
		m_Extension.bIsCreateGuid = TRUE;
		StringFromCLSID(m_Extension.guid, &guidStr);
		StringCbPrintf(szBuffer, dwBufferLen, L"%ws_%ws", LOGGERFILENAME, guidStr);
	}

}

static unsigned int __stdcall ConsumeThreadRoutine(void *pContext)
{
	unsigned int iResult = 0;
	TRACEHANDLE hTrace = *(TRACEHANDLE *)pContext;
	ULONG Status = 0;

	if (hTrace || hTrace != (TRACEHANDLE)INVALID_HANDLE_VALUE)
	{
		Status = ProcessTrace(&hTrace, 1, 0, 0);
	}
	return iResult;
}

UINT32 __stdcall CantiyWmiClass::ProviderThreadRoutine(void *pContext)
{
	BOOL bResult = TRUE;
	EVENT_TRACE_LOGFILE *pLogFile = NULL;
	ULONG BufferLen = sizeof(EVENT_TRACE_LOGFILE);
	TRACEHANDLE traceHandle = 0;
	HANDLE hThread = 0;
	ULONG dwThreadId = 0;

	pLogFile = (PEVENT_TRACE_LOGFILE)malloc(BufferLen);
	ZeroMemory(pLogFile, BufferLen);
	pLogFile->Context = 0;
	pLogFile->LogFileMode = PROCESS_TRACE_MODE_EVENT_RECORD | EVENT_TRACE_REAL_TIME_MODE;
	pLogFile->LoggerName = CantiyWmiClass::ShareInstance()->m_Extension.Properties.m_SessionName;
	pLogFile->EventRecordCallback = (PEVENT_RECORD_CALLBACK)EtwEventTraceRecordCallback;
	pLogFile->BufferCallback = (PEVENT_TRACE_BUFFER_CALLBACK)EventBufferCallbackRoutine;

	traceHandle = OpenTraceW(pLogFile);
	if (traceHandle != (TRACEHANDLE)INVALID_HANDLE_VALUE)
	{
		hThread = (HANDLE)_beginthreadex(0, 0, (_beginthreadex_proc_type)ConsumeThreadRoutine, &traceHandle, 0, (unsigned int *)&dwThreadId);
		CantiyWmiClass::ShareInstance()->m_Extension.hTrace = traceHandle;
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
	else
	{

	}

	CloseTrace(traceHandle);

	if (pLogFile)
		free(pLogFile);
	return bResult;
}

BOOL CantiyWmiClass::CreateEtwEventTrace(void)
{
	TRACE_PROPERTIES Properties;
	HANDLE hThread;
	ULONG dwThreadId;
	HRESULT hr;
	TRACEHANDLE hTrace;
	BOOL bResult = TRUE;

	memset(&Properties, 0, sizeof(TRACE_PROPERTIES));
	CreateGuidStr(Properties.m_SessionName, sizeof(Properties.m_SessionName));
	Properties.m_Properties.Wnode.BufferSize = sizeof(TRACE_PROPERTIES);
	Properties.m_Properties.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
	Properties.m_Properties.Wnode.ClientContext = 2;
	Properties.m_Properties.Wnode.Guid = m_Extension.guid;
	Properties.m_Properties.BufferSize = 0;
	Properties.m_Properties.MaximumBuffers = 0;
	Properties.m_Properties.MinimumBuffers = 0;
	Properties.m_Properties.MaximumFileSize = 0;
	Properties.m_Properties.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
	Properties.m_Properties.FlushTimer = 1;
	Properties.m_Properties.EnableFlags = 0;
	Properties.m_Properties.AgeLimit = 15;
	Properties.m_Properties.LoggerNameOffset = sizeof(Properties.m_Properties);
	m_Extension.Properties = Properties;

	hr = StartTraceW(&hTrace, Properties.m_SessionName, &Properties.m_Properties);

	if (hr == S_OK)
	{
		ENABLE_TRACE_PARAMETERS Parameters = { 0x00 };
		GUID EnableGuid = { 0x00 };
		ULONG Status = S_OK;

		CLSIDFromString(L"{1418EF04-B0B4-4623-BF7E-D74AB47BBDAA}", &EnableGuid);
		m_Extension.ProviderGuid = EnableGuid;
		m_Extension.MatchAndKeyword = 0x8000000000000000;
		Parameters.Version = ENABLE_TRACE_PARAMETERS_VERSION_2;
		Parameters.EnableProperty = 0;
		Parameters.ControlFlags = 0;
		Parameters.SourceId = EnableGuid;
		Parameters.EnableFilterDesc = NULL;
		m_Extension.Parameters = Parameters;
		m_Extension.hSession = hTrace;
		Status = EnableTraceEx2(hTrace, &EnableGuid, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, m_Extension.MatchAndKeyword, 0, INFINITE, &Parameters);
		if (Status == ERROR_SUCCESS)
		{
			hThread = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)ProviderThreadRoutine, NULL, 0, (unsigned int *)&dwThreadId);
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
		else
		{

		}


	}
	else
	{
		bResult = FALSE;
	}

	return bResult;

}

void CantiyWmiClass::CloseTraceAndSession(void)
{
	if (m_Extension.hTrace)
	{
		CloseTrace(m_Extension.hTrace);
	}
	if (m_Extension.hSession)
	{
		ControlTrace(m_Extension.hSession,NULL,&m_Extension.Properties.m_Properties,EVENT_TRACE_CONTROL_STOP);
	}
}

void CantiyWmiClass::UpdateData(void)
{
	if (m_pReport)
	{
		HWND Windows = ::FindWindow(NULL,L"AntiyMonitor-MFC");
		::SendMessage(Windows,WM_UPDATEETWDATA,(WPARAM)m_pReport,UDDM_WMI);
	}
}

BOOL CantiyWmiClass::CreateWmiMon(void)
{
	BOOL bResult = TRUE;
	IWbemLocator *ppv = NULL;
	HRESULT hr;
	OLECHAR *lpObjectName = NULL;

	if (!m_UnsecuredApartment)
	{
		if (CoInitializeEx(0, 0) >= 0)
		{
			if (CoInitializeSecurity(NULL, -1, 0, 0, 0, 3u, 0, 0, 0) < 0 ||
				CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER, riid, (LPVOID *)&ppv) < 0)
			{
				CoUninitialize();
				bResult = FALSE;
				goto ret;
			}

			lpObjectName = SysAllocString(L"ROOT\\Subscription");
			hr = ppv->ConnectServer(lpObjectName, 0, 0, 0, 0, 0, 0, &m_WbemService);
			if (FAILED(hr))
				goto reppv;

			if (CoSetProxyBlanket(m_WbemService, 0xau, 0, NULL, 3u, 3u, NULL, 0) < 0)
			{
				m_WbemService->Release();
			reppv:
				ppv->Release();
				CoUninitialize();
				bResult = FALSE;
				goto ret;
			}


			CoCreateInstance(CLSID_UnsecuredApartment, NULL, 4u, IID_IUnsecuredApartment, (LPVOID *)&m_UnsecuredApartment);

			if (!m_UnsecuredApartment)
			{
				bResult = FALSE;
				goto ret;
			}

			if (!m_WmiListernSink)
			{
				m_WmiListernSink = new EventSink;
				m_WmiListernSink->m_WbemService = m_WbemService;
			}

			m_WmiListernSink->AddRef();
			IUnknown *IUnknowSink = NULL;
			IWbemObjectSink *Resink = NULL;
			hr = m_UnsecuredApartment->CreateObjectStub((IUnknown *)m_WmiListernSink, (IUnknown **)&IUnknowSink);

			IUnknowSink->QueryInterface(IID_IWbemObjectSink, (PVOID *)&Resink);

			/*
				CreateWmiFilter:
				cmd(管理员权限): wmic /NAMESPACE: "\\root\subscription" PATH __EventFilter CREATE Name="BotFilter82",EventNameSpace="root\cimv2",QueryLanguage="WQL", Query="SELECT *FROM __InstanceModificationEvent WITHIN 60 WHERE TargetInstance ISA 'Win32_PerfFormattedData_PerfOS_System'"

				deleteWmiFilter:
				powershell(管理员权限):Get-WMIObject -Namespace root\Subscription -Class __EventFilter -Filter "Name='BotFilter82'" | Remove-WmiObject -Verbose

			*/
			hr = m_WbemService->ExecNotificationQueryAsync(_bstr_t(L"WQL"), _bstr_t(L"SELECT * FROM __InstanceOperationEvent WITHIN 5 WHERE TargetInstance ISA '__EventConsumer' OR TargetInstance I""SA '__EventFilter' OR TargetInstance ISA '__FilterToConsumerBinding'"), 128, NULL, m_WmiListernSink);


			if (FAILED(hr))
			{
				m_WbemService->Release();
				ppv->Release();
				m_UnsecuredApartment->Release();
				Resink->Release();
				m_WmiListernSink->Release();
				IUnknowSink->Release();
				CoUninitialize();
				bResult = FALSE;
			}


		}

	}


ret:
	if (lpObjectName)
	{
		SysFreeString(lpObjectName);
		lpObjectName = NULL;
	}


	return bResult;
}


BOOL CantiyWmiClass::CreateWmiEventMon(VOID)
{
	CreateWmiMon();
	CreateEtwEventTrace();

	return TRUE;
}