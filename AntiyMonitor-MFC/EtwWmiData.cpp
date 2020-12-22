#include <stdio.h>
#include "AntiyWmi.h"
#include <psapi.h>
#include <windows.h>


typedef UINT32(WINAPI *NTQUERYINFORMATIONPROCESS)(HANDLE ProcessHandle,
	ULONG ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
	PULONG ReturnLength);

NTQUERYINFORMATIONPROCESS g_NtQueryInformationProcess = NULL;

typedef struct _PROCESS_BASIC_INFORMATION {
	PVOID Reserved1;
	PVOID PebBaseAddress;
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

CEtwWmiData::CEtwWmiData()
{
	m_Operation = NULL;
	m_ProcessFullPath = NULL;
}

void CEtwWmiData::Add(LPWSTR Key, LPWSTR Value)
{
	UINT32 len = wcslen(Value);
	if (_wcsicmp(Key, L"Operation") == 0)
	{
		m_Operation = new WCHAR[(len + 1) * sizeof(WCHAR)];
		memset(m_Operation, 0x00, len * sizeof(WCHAR));
		wcsncpy_s(m_Operation,len * sizeof(WCHAR), Value, len * sizeof(WCHAR));
	}
}

ULONG GetSymbolicByDosName(WCHAR *szDosName,WCHAR *dwChara)
{
	ULONG Status = 0;
	WCHAR i;
	WCHAR buffer[MAX_PATH] = {0};
	WCHAR CurrentSymbolic[3] = L"C:";
	ULONG dwLen = 0;

	while (1)
	{
		for (i = 'A'; i <= 'Z'; ++i)
		{
			CurrentSymbolic[0] = i;
			Status = QueryDosDevice(CurrentSymbolic,buffer,sizeof(buffer) - sizeof(WCHAR));
			if (Status)
			{
				dwLen = min(wcslen(buffer), wcslen(szDosName));
				if (wcsncmp(buffer, szDosName, dwLen) == 0)
				{
					*dwChara = i;
					break;
				}
			}
			else
			{
				Status = GetLastError();
			}

		}

		break;
	}

	return dwLen;
}

void CEtwWmiData::Add(LPWSTR Key, UINT32 Value)
{
	PROCESS_BASIC_INFORMATION ProcInfo = {0x00};
	ULONG Status = 0;
	ULONG ParentId = 0;

	if (_wcsicmp(Key, L"OperationId") == 0)
	{
		m_OperationId = Value;
	}
	else if (_wcsicmp(Key, L"GroupOperationId") == 0)
	{
		m_GroupOperationId = Value;
	}
	else if (_wcsicmp(Key, L"ClientProcessId") == 0)
	{
		m_ClientProcessid = Value;
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, m_ClientProcessid);
		WCHAR szFullpath[MAX_PATH] = { 0 };

		if (hProcess)
		{
			//GetModuleFileName((HMODULE)hProcess, szFullpath, sizeof(szFullpath) - 1)
			if (!GetProcessImageFileName(hProcess, szFullpath, sizeof(szFullpath)))
			{
				
				Status = GetLastError();
				printf("GetProcessImageFileName fail:%d!\n", Status);
				return;
			}
			WCHAR wcSymbolicName = '0';
			ULONG dwCutLen = 0;

			dwCutLen = GetSymbolicByDosName(szFullpath,&wcSymbolicName);
			ULONG dwLen = (wcslen(szFullpath) - dwCutLen + wcslen(L"C:") + 1) * sizeof(WCHAR);
			m_ProcessFullPath = (WCHAR *)malloc(dwLen);
			memset(m_ProcessFullPath, 0, dwLen);
			*m_ProcessFullPath = wcSymbolicName;
			*(m_ProcessFullPath + 1) = ':';
			CopyMemory(m_ProcessFullPath + 2,szFullpath + dwCutLen,dwLen - 2 * sizeof(WCHAR));

			g_NtQueryInformationProcess = (NTQUERYINFORMATIONPROCESS)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"NtQueryInformationProcess");
			if (g_NtQueryInformationProcess)
			{
				Status = g_NtQueryInformationProcess(hProcess,0,&ProcInfo,sizeof(PROCESS_BASIC_INFORMATION),NULL);
				if (Status >= 0)
				{
					ParentId = (ULONG)ProcInfo.Reserved3;
					HANDLE hParent = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ParentId);
					if (hParent)
					{
						printf("");
						CloseHandle(hParent);
					}
				}
			}

			CloseHandle(hProcess);
		}
		else
		{
			Status = GetLastError();
		}
	}
}

void CEtwWmiData::Out(void)
{
	FILETIME ft, LocalFt;
	SYSTEMTIME st;
	ft.dwLowDateTime = m_TimeStamp.LowPart;
	ft.dwHighDateTime = m_TimeStamp.HighPart;
	FileTimeToLocalFileTime(&ft, &LocalFt);
	FileTimeToSystemTime(&LocalFt, &st);
	printf("Time is %4d-%02d-%02d %02d:%02d:%02d.%d:", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	printf("ClientProcessId is %d,ClientProcess FullPath is %ws,Operation is %ws\n",m_ClientProcessid,m_ProcessFullPath,m_Operation);

}

CEtwWmiData::~CEtwWmiData()
{
	if (m_Operation)
		delete[] m_Operation;
	if (m_ProcessFullPath)
		free(m_ProcessFullPath);
	
}
