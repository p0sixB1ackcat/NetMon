#include <WbemIdl.h>
#include <comutil.h>
#include <in6addr.h>
#include <Psapi.h>
#include "AntiyEtw.h"

IWbemServices  *g_pProxy = NULL;
CAntiyMonLinkList *g_MofLinkListHeader = NULL;

ULONG FormatData(PBYTE MofData, ULONG MofDataLen, CAntiyMonProperty *pProperty, WCHAR *pOutputBuffer, ULONG dwOutputLen);

ULONG
WINAPI
EventTraceLogfileBufferCallback
(
	PEVENT_TRACE_LOGFILEW lpLogfile
	)
{
	return 0;
}

VOID WINAPI EventTraceLogfileEventRecordCallback(PEVENT_RECORD lpRecord)
{
	printf("receive Event Record Call back!\n");
}


WCHAR *GetTcpOperationNameByEventType(UCHAR type)
{
	WCHAR *result = NULL;
	switch (type)
	{
	case EVENT_TRACE_TYPE_ACCEPT:
		result = (WCHAR *)L"TCP Accept";
		break;
	case EVENT_TRACE_TYPE_CONNECT:
	case 28:
		result = (WCHAR *)L"TCP Connect";
		break;
	case EVENT_TRACE_TYPE_DISCONNECT:
	case 29:
		result = (WCHAR *)L"TCP Disconnect";
		break;
	case EVENT_TRACE_TYPE_RECEIVE:
	case 27:
	case 31:
		result = (WCHAR *)L"TCP Receive";
		break;
	case EVENT_TRACE_TYPE_RECONNECT:
	case 32:
		result = (WCHAR *)L"TCP Reconnect";
		break;
	case EVENT_TRACE_TYPE_RETRANSMIT:
	case 30:
		result = (WCHAR *)L"TCP Retransmit";
		break;
	case EVENT_TRACE_TYPE_SEND:
	case 26:
		result = (WCHAR *)L"TCP Send";
		break;

	case 17:
		//fail
		break;
	case 18:
	case 34:
		result = (WCHAR *)L"Tcp Copy";
		break;
	default:
		result = (WCHAR *)L"Unknow";
	}
	return result;
}

WCHAR *GetUdpOperationByEventType(UCHAR type)
{
	WCHAR *result = NULL;
	switch (type)
	{
	case EVENT_TRACE_TYPE_RECEIVE:
	case 27:
		result = (WCHAR *)L"UDP Receive";
		break;
	case EVENT_TRACE_TYPE_SEND:
	case 26:
		result = (WCHAR *)L"UDP Send";
		break;
	case 17:
		//fail
		break;
	default:
		result = (WCHAR *)L"Unknow";
		break;
	}
	return result;
}

BOOL GetProcessNameByPid(ULONG dwPid)
{
	HANDLE hProcessHandle = 0;
	WCHAR ProcessFullPath[MAX_PATH] = {0x00};
	hProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwPid);
	BOOL bResult = FALSE;
	if (hProcessHandle != 0 && hProcessHandle != INVALID_HANDLE_VALUE)
	{
		if (GetProcessImageFileName(hProcessHandle, ProcessFullPath, sizeof(ProcessFullPath)))
		{
			bResult = TRUE;
			WCHAR *p, *q;
			p = q = ProcessFullPath;
			while (*p != '\0')
			{
				if (*p == '\\')
				{
					q = p;
				}
				++p;
			}
			CopyMemory(CAntiyEventTraceData::ShareInstance()->m_ProcessName,++q,sizeof(CAntiyEventTraceData::ShareInstance()->m_ProcessName));
		}
	}
	return bResult;
}

VOID DisposeDataStamp(LARGE_INTEGER stamp)
{
	LARGE_INTEGER finalTime;
	FILETIME ft;
	finalTime = CalcTime(stamp);
	ft.dwLowDateTime = finalTime.LowPart;
	ft.dwHighDateTime = finalTime.HighPart;
	GetPointerByFileTime(&ft);

}

VOID WINAPI EventTraceLogfileEventCallback(PEVENT_TRACE lpTrace)
{
	ULONG time = 0;
	IEnumWbemClassObject *lpEnumClass = NULL;
	IWbemClassObject *lpClass = NULL;
	HRESULT hr = 0;
	IWbemQualifierSet *lpClassQualifierSet = NULL;
	HRESULT hQualifier = 0;
	VARIANT varVar = { 0x00 };
	WCHAR wBuffer[50];
	ULONG cnt;
	VARIANT varVersion;
	BOOL bEnumResult = FALSE;
	CAntiyMonDataEntry *pEventData = NULL;
	if (!lpTrace)
		return;

	if (!g_MofLinkListHeader)
	{
		g_MofLinkListHeader = new CAntiyMonLinkList(NULL, NULL);
	}

 	pEventData = ResolveEventTrace(lpTrace);
	
	if (!pEventData->m_PropertyList)
		return;
	LIST_ENTRY *pEntry = pEventData->m_PropertyList->m_Entry.Flink;
	PBYTE pMofData = (PBYTE)lpTrace->MofData;
	ULONG dwPid = 0;
	ULONG dwSize = 0;
	ULONG dwDaddr = 0;
	ULONG dwSaddr = 0;
	USHORT dwDport = 0;
	USHORT dwSport = 0;
	BOOL isUpdate = TRUE;
	while (pEntry != &pEventData->m_PropertyList->m_Entry)
	{
		CAntiyMonLinkList *pLink = CONTAINING_RECORD(pEntry, CAntiyMonLinkList, m_Entry);
		if (pLink)
		{
			CAntiyMonProperty *pProperty = (CAntiyMonProperty *)pLink->m_pDataEvent;

			if (pProperty)
			{
				if (_wcsicmp(pProperty->m_Data->m_Data, L"PID") == 0)
				{
					CopyMemory(&dwPid, pMofData, sizeof(ULONG));
					if (dwPid == 4 || dwPid == 0)
					{
						isUpdate = FALSE;
						break;
					}
					swprintf_s(CAntiyEventTraceData::ShareInstance()->m_ProcessId,sizeof(CAntiyEventTraceData::ShareInstance()->m_ProcessId), L"%d", dwPid);
					GetProcessNameByPid(dwPid);
					pMofData += sizeof(ULONG);
				}
				else if (_wcsicmp(pProperty->m_Data->m_Data, L"size") == 0)
				{
					CopyMemory(&dwSize, pMofData, sizeof(ULONG));
					//printf("size is %d ", dwSize);
					pMofData += 4;
				}
				else if (_wcsicmp(pProperty->m_Data->m_Data, L"daddr") == 0)
				{
					CopyMemory(&dwDaddr, pMofData, sizeof(ULONG));

					pMofData += FormatData(pMofData, lpTrace->MofLength, pProperty, CAntiyEventTraceData::ShareInstance()->m_RemoteIpAddress, sizeof(CAntiyEventTraceData::ShareInstance()->m_RemoteIpAddress));

				}

				else if (_wcsicmp(pProperty->m_Data->m_Data, L"saddr") == 0)
				{
					CopyMemory(&dwSaddr, pMofData, sizeof(ULONG));
					//pMofData += sizeof(ULONG);

					pMofData += FormatData(pMofData, lpTrace->MofLength, pProperty, CAntiyEventTraceData::ShareInstance()->m_LocalIpAddress, sizeof(CAntiyEventTraceData::ShareInstance()->m_LocalIpAddress));
					
				}
				else if (_wcsicmp(pProperty->m_Data->m_Data, L"dport") == 0)
				{
					dwDport = ntohs(*((USHORT *)pMofData));
					//CopyMemory(&dwDport, pMofData ,sizeof(USHORT));
					pMofData += sizeof(USHORT);
					CAntiyEventTraceData::ShareInstance()->m_RemotePort = dwDport;
				}
				else if (_wcsicmp(pProperty->m_Data->m_Data, L"sport") == 0)
				{
					dwSport = ntohs(*((USHORT *)pMofData));
					//CopyMemory(&dwSport, pMofData, sizeof(USHORT));
					pMofData += sizeof(USHORT);
					CAntiyEventTraceData::ShareInstance()->m_LocalPort = dwSport;
				}
				//printf("\n");

			}
		}
		pEntry = pEntry->Flink;
	}

	if (isUpdate)
	{
		swprintf_s(CAntiyEventTraceData::ShareInstance()->m_OperationPath, L"%ws:%u -> %ws:%u", CAntiyEventTraceData::ShareInstance()->m_LocalIpAddress, CAntiyEventTraceData::ShareInstance()->m_LocalPort, CAntiyEventTraceData::ShareInstance()->m_RemoteIpAddress, CAntiyEventTraceData::ShareInstance()->m_RemotePort);
		if (_wcsicmp(pEventData->m_ClassData->m_Data, L"TcpIp") == 0)
		{
			CopyMemory(CAntiyEventTraceData::ShareInstance()->m_OperType, GetTcpOperationNameByEventType(lpTrace->Header.Class.Type), sizeof(CAntiyEventTraceData::ShareInstance()->m_OperType));
		}
		else if (_wcsicmp(pEventData->m_ClassData->m_Data, L"UdpIp") == 0)
		{
			CopyMemory(CAntiyEventTraceData::ShareInstance()->m_OperType, GetUdpOperationByEventType(lpTrace->Header.Class.Type), sizeof(CAntiyEventTraceData::ShareInstance()->m_OperType));
		}
		if (wcslen(CAntiyEventTraceData::ShareInstance()->m_OperationPath) > 0)
		{
			CopyMemory(CAntiyEventTraceData::ShareInstance()->m_OperaResult,L"SUCCESS",sizeof(CAntiyEventTraceData::ShareInstance()->m_OperaResult));
		}
		DisposeDataStamp(lpTrace->Header.TimeStamp);
		CAntiyEventTraceData::ShareInstance()->m_EtwCallback();
	}

	
}

ULONG FormatData(PBYTE MofData, ULONG MofDataLen, CAntiyMonProperty *pProperty, WCHAR *pOutputBuffer, ULONG dwOutputLen)
{
	ULONG dwArrayLen = pProperty->m_Len;
	ULONG dwResult = 0;

	switch (dwArrayLen)
	{
	case 20:
	{
		swprintf_s(pOutputBuffer, dwOutputLen, L"%3d.%3d.%3d.%3d", (unsigned __int8)*(ULONG *)MofData, (unsigned __int16)*(ULONG *)MofData >> 8, (*(ULONG *)MofData >> 16) & 0xff, *(ULONG *)MofData >> 24);
		dwResult = 4;
	}
	break;
	case 27:
	{
		/*swprintf_s(pOutputBuffer, dwOutputLen, L"%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", *MofData, *(MofData + 1), *(MofData + 2), *(MofData + 3), *(MofData + 4), *(MofData + 5), *(MofData + 6), *(MofData + 7), *(MofData + 8), *(MofData + 9), *(MofData + 10), *(MofData + 11), *(MofData + 12), *(MofData + 13), *(MofData + 14), *(MofData + 15));*/
		swprintf_s(pOutputBuffer, dwOutputLen, L"%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x", *MofData, *(MofData + 1), *(MofData + 2), *(MofData + 3), *(MofData + 4), *(MofData + 5), *(MofData + 6), *(MofData + 7), *(MofData + 8), *(MofData + 9), *(MofData + 10), *(MofData + 11), *(MofData + 12), *(MofData + 13), *(MofData + 14), *(MofData + 15));
		dwResult = 16;
	}
	break;
	}

	return dwResult;
}

//当会话启用或禁用您的提供程序时，ETW调用的回调通知您。可以为 NULL。
VOID
NTAPI
RegEventCallBack
(
	_In_ LPCGUID SourceId,
	_In_ ULONG IsEnabled,
	_In_ UCHAR Level,
	_In_ ULONGLONG MatchAnyKeyword,
	_In_ ULONGLONG MatchAllKeyword,
	_In_opt_ PEVENT_FILTER_DESCRIPTOR FilterData,
	_Inout_opt_ PVOID CallbackContext
	)
{
	printf("RegEventCallback:%d!\n", *(int *)CallbackContext);
}

SERVICE_STATUS_HANDLE WINAPI StartServiceCtrlDispatchCallback
(
	DWORD   dwNumServicesArgs,
	LPWSTR  *lpServiceArgVectors
	)
{
	SERVICE_STATUS_HANDLE hService = 0;
	//LPCWSTR lpServiceName = lpSourceName;
	LPHANDLER_FUNCTION_EX callback = RegServiceCtrlHandlerExWCallback;
	//hService = RegisterServiceCtrlHandlerExW(lpServiceName, RegServiceCtrlHandlerExWCallback, 0);

	return hService;
}

DWORD WINAPI RegServiceCtrlHandlerExWCallback
(
	DWORD    dwControl,
	DWORD    dwEventType,
	LPVOID   lpEventData,
	LPVOID   lpContext
	)
{
	printf("%s!\n", __FUNCDNAME__);
	return NO_ERROR;
}